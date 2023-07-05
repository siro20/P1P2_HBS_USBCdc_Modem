#pragma once
#include "fifo_irqsafe.hpp"
#include "message.hpp"

#include "pico/stdlib.h"
#include "hardware/pio.h"

// High level abstraction of P1P2 UART transmit functions
class UARTPio
{
	public:
	static UARTPio& getInstance(void)
	{
		__scratch_y("UARTPioInstance") static UARTPio instance;
		return instance;
	}

	UARTPio(UARTPio const&) = delete;
	void operator=(UARTPio const&) = delete;

	UARTPio(void);

	bool Transmitting(void);
	void Send(const Message& m);
	bool DataWaiting(void);
	void EnableShutdown(bool state);
	void ClearFifo(void);
	bool Error(void);

	private:
	bool error;
	PIO pio;
	uint sm;
	// DMA channels
	int channel;

	// Pin2 and Pin3 are used for P1P2 transmission
	// Pin20 sets the transmitting into shutdown mode
	static const uint PIN_UP = 2;
	static const uint PIN_DOWN = 3;
	static const uint PIN_SHUTDOWN = 20;
	uint8_t Data[32];
};
