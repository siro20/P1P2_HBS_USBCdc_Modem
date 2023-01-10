#include <inttypes.h>
#include "uart_pio.hpp"
#include "p1p2_uart_tx.pio.h"
#include "hardware/dma.h"

#include <iostream>

// Initialize PIO0 SM0 to generate the P1P2 bus encoded serial UART.
// The serial runs at 9600 baud, parity even, 1 stop bit.
UARTPio::UARTPio() : pio(pio0), sm(0), channel(0), Data{}
{
	uint offset = pio_add_program(this->pio, &p1p2_uart_tx_program);
	p1p2_uart_tx_program_init(this->pio, this->sm, offset, UARTPio::PIN_UP, 9600);

	// Get a free channel, panic() if there are none
	this->channel = dma_claim_unused_channel(true);

	dma_channel_config c = dma_channel_get_default_config(this->channel);
	// 8 bit transfers
	channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
	// increment the write adddress, don't increment read address
	channel_config_set_read_increment(&c, true);
	channel_config_set_write_increment(&c, false);

	channel_config_set_high_priority(&c, false);

	// Pace transfers based on availability of TX FIFO not full
	channel_config_set_dreq(&c, pio_get_dreq(this->pio, this->sm, true));

	dma_channel_configure(this->channel, &c,
		p1p2_uart_tx_reg(this->pio, this->sm) ,    // dst
		this->Data,                                // src
		0,                                         // transfer count
		false                                      // start immediately
	);

	gpio_init(UARTPio::PIN_SHUTDOWN);
	gpio_set_dir(UARTPio::PIN_SHUTDOWN, true);
	gpio_put(UARTPio::PIN_SHUTDOWN, false);
}

// Power up/down the hardware.
// Note: It make take some time for the changes to take effect!
void UARTPio::EnableShutdown(bool state) {
	gpio_put(UARTPio::PIN_SHUTDOWN, state);
}

// Transmitting returns true as long as data is being transmitted or the TX FIFO is not empty
bool UARTPio::Transmitting(void) {
	return pio_interrupt_get(pio, sm) ||
		!pio_sm_is_tx_fifo_empty(this->pio, this->sm) ||
		this->DataWaiting();
}

// ClearFifo discards all data stored in the TX FIFO
void UARTPio::ClearFifo(void) {
	dma_channel_abort(this->channel);
	pio_sm_clear_fifos(this->pio, this->sm);
}

// Transmit the message on the bus.
// Does not check for bus being idle or bus collisions!
void UARTPio::Send(const Message& m) {
	if (dma_channel_is_busy(this->channel)) {
		this->error = true;
		return;
	}
	if (m.Length > sizeof(this->Data)) {
		this->error = true;
		return;
	}
	for (int i = 0; i < m.Length; i++) {
		this->Data[i] = m.Data[i];
	}

	dma_channel_set_read_addr(this->channel, this->Data, false);
	dma_channel_set_trans_count(this->channel, m.Length, false);

	dma_start_channel_mask(1 << this->channel);
}

// Returns true if the FIFO has data
bool UARTPio::DataWaiting(void) {
	return dma_channel_is_busy(this->channel);
}

// Returns true if a buffer overrun was detected
bool UARTPio::Error(void) {
	bool tmp = this->error;
	this->error = false;
	return tmp;
}