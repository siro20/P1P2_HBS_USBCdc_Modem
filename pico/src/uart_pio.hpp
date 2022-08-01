#include "circular_buffer_spinlock.hpp"

#include "pico/stdlib.h"
#include "hardware/pio.h"

// High level abstraction of P1P2 UART transmit functions
class UARTPio
{
	public:
	static UARTPio& getInstance(void)
	{
		static UARTPio instance;
		return instance;
	}

	UARTPio(UARTPio const&) = delete;
	void operator=(UARTPio const&) = delete;

	UARTPio(void);

	bool Transmitting(void);
	void Send(const uint8_t data);
	void DrainSWFifo(void);
	bool DataWaiting(void);
	void EnableShutdown(bool state);
	void ClearFifo(void);

	private:
	PIO pio;
	uint sm;
	// Pin2 and Pin3 are used for P1P2 transmission
	// Pin20 sets the transmitting into shutdown mode
	static const uint PIN_UP = 2;
	static const uint PIN_DOWN = 3;
	static const uint PIN_SHUTDOWN = 20;
	CircularBufferSpinlock<uint8_t, 32>fifo;
};
