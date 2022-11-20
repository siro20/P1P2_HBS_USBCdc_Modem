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
__scratch_x("ADCInstance") int16_t adc_data[0x100] __attribute__ ((aligned(0x200)));
DifferentialADC& dadc = DifferentialADC::getInstance(adc_data);

// filter implements a FIR filter to reduce high frequency noise
// captured by the ADC. As the FIR filter is processing intense,
// the length was set to 7.
// Introduces a delay of about 4 ADC samples.
__scratch_x("FIRFilter1") int32_t fir_data1[7 * 2];
__scratch_x("FIRFilter2") int32_t fir_data2[7 * 2];
FIRFilter filter(fir_data1, fir_data2);

#if (FIR_OVERSAMPLING_RATE / UART_OVERSAMPLING_RATE) == 4
// resampler drops 3 samples out of 4 as it's not needed any more after
// FIR filtering the signal.
// Provides an 8x oversampled signal.
Resample<int32_t> resampler(3);
#else
#if (FIR_OVERSAMPLING_RATE / UART_OVERSAMPLING_RATE) == 2
// resampler drops 1 samples out of 2 as it's not needed any more after
// FIR filtering the signal.
// Provides an 8x oversampled signal.
Resample<int32_t> resampler(1);
#else
#error Unsupported FIR_OVERSAMPLING_RATE to UART_OVERSAMPLING_RATE ratio!
#endif
#endif

// dcblock removes the DC level by using about 200 samples. DC offsets can
// appear when the used resistors have a high tolerance and don't properly
// match each others value.
DCblock dcblock;

// p1p2uart decodes the P1P2 data signal to bytes. It can detect parity errors, frame
// errors and DC errors ("0" not encoded as alternating up/down).
__scratch_x("UART") int16_t uart_data[UART_BUFFER_LEN * 2];
UART p1p2uart(uart_data, UART::PARITY_EVEN);

// Level applies the P1P2 bus hysteresis.
// The low level signal amplitude is reduced to 0.1.
// The high level signal amplitude is unchanged.
Level<int32_t> level;

// bit returns the probabilty for a high or low pulse found in the signal.
// Allow 0xE0/0x100 bit errors = 12,5%
__scratch_x("UARTBit1") int32_t uart_bit_data[UART_OVERSAMPLING_RATE * 2];
__scratch_x("UARTBit2") int32_t uart_bit_data2[UART_OVERSAMPLING_RATE * 2];
UARTBit<int32_t, UART_OVERSAMPLING_RATE> bit(uart_bit_data, uart_bit_data2, BUS_HIGH_MV, BUS_LOW_MV, 0xE0);

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

union CoreInterchangeData {
	struct {
		uint8_t RxChar;
		uint8_t RxValid : 1;
		uint8_t RxError : 1;
		uint8_t LineBusy : 1;
		uint8_t LineFree : 1;
		uint8_t DACError : 1;
	};
	uint32_t Raw;
};

// Data exchange variables. Unidirectional only.
volatile bool FifoErr;

enum TRANSMITTER_STATE {
	TX_IDLE,
	TX_STARTED_WAIT_FOR_BUSY,
	TX_RUNNING_CHECK_DATA,
	TX_RUNNING_WAIT_FOR_IDLE
};

static void core1_entry() {
	uint8_t rx_data;
	bool rx_error;
	int32_t adc_data, fir_data, resamp_data, ac_data, hysteresis_data, bit_data;
	int32_t LineIdleCounter;
	bool LineIsBusy;

	CoreInterchangeData Core1Data;

	FifoErr = false;
	LineIsBusy = false;
	Core1Data.Raw = 0;
	LineIdleCounter = 0;

	dadc.SetGain((uint16_t)(ADC_EXTERNAL_GAIN * 0x100));
	dadc.Start();

	for (;;) {
		if (Core1Data.Raw) {
			if (!multicore_fifo_wready()) {
				FifoErr = true;
			} else {
				multicore_fifo_push_timeout_us(Core1Data.Raw, 0);
			}
			Core1Data.Raw = 0;
		}
		if(!dadc.Update(&adc_data)) {
			__wfe();
			continue;
		}
		if (dadc.Error ()) {
			Core1Data.DACError = true;
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

		bit.Update(hysteresis_data, &bit_data);

		// p1p2uart is busy as long as receiving a byte. It has an
		// idle phase of UART_OVERSAMPLING_RATE/2 or less between two bytes.
		// Thus the line is idle when p1p2uart haven't signaled busy for
		// at least UART_OVERSAMPLING_RATE samples.
		//
		// The idle time between packets on the P1/P2 bus is unknown.
		// Assume idle time of 1 byte == 9600Baud/11 bits == 1.15msec.

		if (p1p2uart.Receiving()) {
			if (!LineIsBusy) {
				LineIsBusy = true;
				LineIdleCounter = 0;
				Core1Data.LineBusy = 1;
			}
		} else if (LineIdleCounter > 0) {
			LineIdleCounter--;
		} else if (LineIsBusy) {
			LineIsBusy = false;
			Core1Data.LineFree = 1;
		}

		rx_data = 0;
		if (!p1p2uart.Update(bit_data, &rx_data, &rx_error)) {
			continue;
		}
		Core1Data.RxChar = rx_data;
		Core1Data.RxError = rx_error;
		Core1Data.RxValid = !rx_error;

		// Set the line idle counter. p1p2uart no longer singals Busy at this
		// points so timeout.
		LineIdleCounter = 11 * UART_OVERSAMPLING_RATE;
	}
}

static void core0_entry() {
	Message RxMsg;
	Message TxMsg;
	size_t TxOffset;
	bool BusCollision;

	CoreInterchangeData Core1Data;
	bool LineIsBusy;
	enum TRANSMITTER_STATE TxState;

	LineIsBusy = false;
	TxState = TX_IDLE;
	TxOffset = 0;
	BusCollision = false;

	// discard old data
	multicore_fifo_drain();

	while(1) {
		if (multicore_fifo_rvalid()) {
			Core1Data.Raw = multicore_fifo_pop_blocking();
		} else {
			__wfe();
			continue;
		}
		// Update RxMsg
		if (Core1Data.DACError)
			RxMsg.Status = Message::STATUS_ERR_OVERFLOW;
		else if (Core1Data.RxError)
			RxMsg.Status = Message::STATUS_ERR_PARITY;
		else if (Core1Data.RxValid)
			RxMsg.Append(Core1Data.RxChar);
		// Packet end reached, transmit now...
		if (Core1Data.LineFree) {
			if (RxMsg.Length > 0 || RxMsg.Status != 0) {
				hostUart.UpdateAndSend(RxMsg);
				RxMsg.Clear();
			}
		}
		// Received more data than would fit into message...
		if (RxMsg.Overflow()) {
			RxMsg.Status = Message::STATUS_ERR_OVERFLOW;
			hostUart.UpdateAndSend(RxMsg);
			RxMsg.Clear();
		}

		// Update half duplex state machine
		if (Core1Data.LineFree && LineIsBusy) {
			LineIsBusy = false;
		} else if (Core1Data.LineBusy && !LineIsBusy) {
			LineIsBusy = true;
		}

		switch (TxState) {
		case TX_IDLE:
			if (!LineIsBusy && !uart_tx.Transmitting() && hostUart.HasData()) {
				TxState = TX_STARTED_WAIT_FOR_BUSY;
				TxMsg = hostUart.Pop();
				uart_tx.EnableShutdown(false);
				uart_tx.Send(TxMsg);
				TxOffset = 0;
			}
		break;
		case TX_STARTED_WAIT_FOR_BUSY:
			if (Core1Data.RxError)
				BusCollision = true;

			if (LineIsBusy || Core1Data.RxValid)
				TxState = TX_RUNNING_CHECK_DATA;
			if (!Core1Data.RxValid)
				break;

		case TX_RUNNING_CHECK_DATA:
			if (Core1Data.RxError)
				BusCollision = true;
			else if (Core1Data.RxValid) {
				if(TxMsg.Data[TxOffset] != Core1Data.RxChar)
					BusCollision = true;
				TxOffset++;
				if (TxOffset == TxMsg.Length)
					TxState = TX_RUNNING_WAIT_FOR_IDLE;
			}
		break;
		case TX_RUNNING_WAIT_FOR_IDLE:
			// The RX state machine will insert the neccessary delay between
			// two packets. No need to wait here.
			if (!LineIsBusy) {
				TxMsg.Clear();
				TxState = TX_IDLE;
			}
			if (!uart_tx.Transmitting()) {
				uart_tx.EnableShutdown(true);
			}
		break;
		}

		if (BusCollision) {
			BusCollision = false;
			uart_tx.ClearFifo();
			TxState = TX_RUNNING_WAIT_FOR_IDLE;
		}

		// Update LEDs

		if (Core1Data.RxError) {
			LedManager.TransmissionErrorRx();
			printf("c ");

		} else if (Core1Data.RxValid) {
			LedManager.ActivityRx();
		//	printf("%02x ", Core1Data.RxChar);
		}
	
		if (uart_tx.Transmitting()) {
			LedManager.ActivityTx();
		}
		if (Core1Data.DACError) {
			printf("d ");
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

		Core1Data.Raw = 0;
	};
}

int main(void) {
	stdio_init_all();	// Must be called on core0!
	sleep_ms(3000);

	printf("\n#==================\n");
	printf("#RP2040 P1/P2 modem\n");
	printf("#==================\n");
	uart_default_tx_wait_blocking();

	multicore_launch_core1(core1_entry);

	core0_entry();
}
