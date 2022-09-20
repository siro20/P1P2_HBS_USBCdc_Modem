#include <stdio.h>
#include <pico/platform.h>
#include <pico/stdlib.h>
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

// gain adjust the signal amplitude to 2*P1/P2 bus high level = 2.8V.
// For transmitters driving higher voltage pulses on the bus, this also
// improves bus idle detection by lowering the overshoot after a "high"
// symbol.
__scratch_x("AGC") AGC gain;

// p1p2uart decodes the P1P2 data signal to bytes. It can detect parity errors, frame
// errors and DC errors ("0" not encoded as alternating up/down).
__scratch_x("UART")  UART p1p2uart(800, UART::PARITY_EVEN);

// pio implements the P1P2 transmitting part. The caller must avoid bus collisions on
// the half duplex P1P2 bus. pio has an internal 64 byte software fifo.
UARTPio& uart_tx = UARTPio::getInstance();

// HostUART implement the logic to interface with the host
// It generates and decodes the messages transceived on the host interfaces.
// It keeps an internal time and update the status bits on internal error.
//HostUART& hostUart = HostUART::getInstance();

// busy gives an approximation if the line is currently in use.
LineBusy<16> busy(BUS_HIGH_MV*2/2);

// LED drivers
LEDdriver PowerLED = LEDdriver(21);
LEDdriver TxLED = LEDdriver(19);
LEDdriver RxLED = LEDdriver(18);

struct csv {
	int16_t sample;
	uint8_t flags;
	int16_t adca;
	int16_t adcb;
};

int main(void) {
	uint8_t rx_data;
	bool rx_error;
	int32_t adc_data, fir_data, resamp_data, compensated_data, gain_data, ac_data;
	//struct csv samples[2048];
	int i = 0, j = 0;
	Message rx;
	int16_t h, l;
	bool now = true;

	stdio_init_all();

	sleep_ms(3000);
	PowerLED.Set(LEDdriver::LED_ON);
	TxLED.Set(LEDdriver::LED_BLINK_SLOW);
	RxLED.Set(LEDdriver::LED_BLINK_FAST);

	printf("\n===========================\n");
	printf("RP2040 ADC and Test Console\n");
	printf("===========================\n");
	uart_default_tx_wait_blocking();

	dadc.SetGain((uint16_t)(1.51 * 0x100));
	dadc.Start();

	for (;;) {
		//__wfe();
#if 1
		if (now) {
			uart_tx.Send(j++);

			now = false;
		}
#endif
		if(!dadc.Update(&adc_data)) {
			continue;
		}
		if (dadc.Error ()) {
			printf("DAC overrun\n");
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

		//if (!comp.Update(resamp_data, &compensated_data)) {
		//	continue;
		//}

	i++;
	if (i == 4096) {
		i = 0;
		now = true;
	}

		//if (!gain.Update(resamp_data, &gain_data)) {
		//	continue;
		//}
		// Detect line idle here for packet frame detection
		//busy.Update(resamp_data);
		// Detect packet frame and move to other core if ready
#if 0
		if (rx.Length > 0) {
			if (busy.Busy())
				rx.Time = time_us_64();
			else if (time_us_64() - rx.Time > 520) {// 1041usec == 9600 baud * 10symbols. FIXME!
				// TODO: send to other core
				//printf("%s", rx.c_str());
				rx.Length = 0;
			}
		}
#endif
		rx_data = 0;
		if (!p1p2uart.Update(ac_data, &rx_data, &rx_error)) {
			continue;
		}
		if (!rx_error) {
			rx.Data[rx.Length] = rx_data;
			rx.Length++;
			rx.Time = time_us_64();
			if (rx.Length == 32) {
				//for (int i = 0; i < 32; i++)
				//	printf("%02x ", rx.Data[i]);
				rx.Length = 0;
				//while(1);
			}
		}
	}

}
