#include <stdio.h>
#include <pico/platform.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>

#include <hardware/sync.h>

#include "adc.hpp"
#include "uart.hpp"
#include "fir_filter.hpp"
#include "resample.hpp"
#include "uart_pio.hpp"
#include "led_driver.hpp"
#include "host_uart.hpp"
#include "line_busy.hpp"
#include "channel_comp.hpp"
#include "gain.hpp"
#include "dcblock.hpp"
#include "led_manager.hpp"
#include "level_detect.hpp"
#include "uart_bit_detect_fast.hpp"

//
// Global signal processing blocks
//

// DifferentialADC provides the differential AC voltage signal captured from
// the P1P2 lines. As the RP2040 has no differential ADC hardware, the
// signal is delayed by 2 ADC samples to emulate a phase correct differential
// capture.
// Provides an 16x oversampled signal.
__scratch_x("ADC") DifferentialADC& dadc = DifferentialADC::getInstance();

// filter implements a FIR filter to reduce high frequency noise
// captured by the ADC. As the FIR filter is processing intense,
// the length was set to 7.
// Introduces a delay of about 4 ADC samples.
__scratch_x("FIRFilter") FIRFilter filter;

#if (FIR_OVERSAMPLING_RATE / UART_OVERSAMPLING_RATE) == 4
// resampler drops 3 samples out of 4 as it's not needed any more after
// FIR filtering the signal.
// Provides an 8x oversampled signal.
__scratch_x("Resample") Resample<int32_t> resampler(3);
#else
#if (FIR_OVERSAMPLING_RATE / UART_OVERSAMPLING_RATE) == 2
// resampler drops 1 samples out of 2 as it's not needed any more after
// FIR filtering the signal.
// Provides an 8x oversampled signal.
__scratch_x("Resample") Resample<int32_t> resampler(1);
#else
#error Unsupported FIR_OVERSAMPLING_RATE to UART_OVERSAMPLING_RATE ratio!
#endif
#endif

// comp reduces the distortions introduced by the AC coupling and bias
// circuit on the receiver side. This reduces the overshoot in the idle phase.
__scratch_x("ChannelComp") ChannelComp comp;

// dcblock removes the DC level by using about 200 samples. DC offsets can
// appear when the used resistors have a high tolerance and don't properly
// match each others value.
__scratch_x("DCblock") DCblock dcblock;

// p1p2uart decodes the P1P2 data signal to bytes. It can detect parity errors, frame
// errors and DC errors ("0" not encoded as alternating up/down).
UART p1p2uart(UART::PARITY_EVEN);

// busy gives an approximation if the line is currently in use.
__scratch_x("Busy") LineBusy<16> busy(BUS_HIGH_MV*2/2);

// Level applies the P1P2 bus hysteresis.
// The low level signal amplitude is reduced to 0.1.
// The high level signal amplitude is unchanged.
__scratch_x("Level") Level<int32_t> level;

// bit returns the probabilty for a high or low pulse found in the signal.
// Allow 0xE0/0x100 bit errors = 12,5%
UARTBit<int32_t, UART_OVERSAMPLING_RATE> bit(BUS_HIGH_MV, BUS_LOW_MV, 0xE0);

// uart_tx implements the P1P2 transmitting part. The caller must avoid bus collisions on
// the half duplex P1P2 bus. uart_tx has an internal 64 byte software fifo.
__scratch_y("UART") UARTPio& uart_tx = UARTPio::getInstance();

// HostUART implement the logic to interface with the host
// It generates and decodes the messages transceived on the host interfaces.
// It keeps an internal time and update the status bits on internal error.
__scratch_y("host_uart") HostUART& hostUart = HostUART::getInstance();

// LED drivers
__scratch_y("pled") LEDdriver PowerLED = LEDdriver(21);
__scratch_y("tled") LEDdriver TxLED = LEDdriver(19);
__scratch_y("rled") LEDdriver RxLED = LEDdriver(18);

// The LED manager drives various patterns on the LEDs depending on the current
// system state.
__scratch_y("ledmanager") LEDManager LedManager(RxLED, TxLED, PowerLED);

struct csv {
	int16_t sample;
	uint8_t flags;
	int16_t adca;
	int16_t adcb;
};

volatile bool FifoErr;
volatile bool DADCErr;

static void core1_entry() {
	// Millisecond tracking
	uint32_t ms_since_boot, ms_now;
	// Increments from 0 to 1000
	uint32_t second_cnt;
	int j = 0;
	sleep_ms(3000);

	printf("\n===========================\n");
	printf("RP2040 ADC and Test Console\n");
	printf("===========================\n");
	uart_default_tx_wait_blocking();

	second_cnt = 0;
	while(1) {
		//__wfe();

		ms_now = to_ms_since_boot(get_absolute_time());
		if (ms_now != ms_since_boot) {
			ms_since_boot = ms_now;
			second_cnt++;
		}

		if (second_cnt == 2) {
			uart_tx.Send(j++);
			second_cnt = 0;
		}

		if (multicore_fifo_rvalid()) {
			uint32_t data = multicore_fifo_pop_blocking();
			LedManager.ActivityRx();

			// Fixme bus collision?
			if (data & (1 << 8)) {
				LedManager.TransmissionErrorRx();
				printf("c ");

			}
			//printf("%02x ", data & 0xff);
		}
		if (uart_tx.Transmitting()) {
			LedManager.ActivityTx();
		}
		if (DADCErr) {
			printf("d ");
			DADCErr = false;
			LedManager.InternalError();
		}
		if (FifoErr) {
			printf("f ");
			FifoErr = false;
			LedManager.InternalError();
		}
		if (uart_tx.Error()) {
			printf("u ");

			LedManager.InternalError();
		}
	};
}


int main(void) {
	uint8_t rx_data;
	uint32_t fifo_data;
	bool rx_error;
	int32_t adc_data, fir_data, resamp_data, ac_data, hysteresis_data, bit_data;

	stdio_init_all();

	multicore_launch_core1(core1_entry);

	DADCErr = false;
	FifoErr = false;

	dadc.SetGain((uint16_t)(ADC_EXTERNAL_GAIN * 0x100));
	dadc.Start();
	for (;;) {	
		if(!dadc.Update(&adc_data)) {
			__wfe();
			continue;
		}
		if (dadc.Error ()) {
			DADCErr = true;
			dadc.Reset();
			continue;
		}
		if (!filter.Update(adc_data, &fir_data)) {
			continue;
		}
		if (!resampler.Update(fir_data, &resamp_data)) {
			continue;
		}
		if (!dcblock.Update(resamp_data, &ac_data)) {
			continue;
		}
		if (!level.Update(ac_data, &hysteresis_data)) {
			continue;
		}

		// Detect line-idle here for packet frame detection
		//busy.Update(resamp_data)

		bit.Update(hysteresis_data, &bit_data);

		rx_data = 0;
		if (!p1p2uart.Update(bit_data, &rx_data, &rx_error)) {
			continue;
		}

		fifo_data = rx_data;
		if (rx_error)
			fifo_data |= 1 << 8;

		if (!multicore_fifo_wready()) {
			FifoErr = true;
			continue;
		}
		multicore_fifo_push_timeout_us(fifo_data, 0);
	}

}
