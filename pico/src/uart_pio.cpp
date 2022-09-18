#include <inttypes.h>
#include "uart_pio.hpp"
#include "p1p2_uart_tx.pio.h"

#include <iostream>

static void pio_irq0_tx_fifo_not_full_handler()
{
	UARTPio& pio = UARTPio::getInstance();
	// Try to move data from SW FIFO to HW FIFO
	pio.DrainSWFifo();

	pio_interrupt_clear(pio0, pis_sm0_tx_fifo_not_full);
	irq_clear(PIO0_IRQ_0);
}

// Initialize PIO0 SM0 to generate the P1P2 bus encoded serial UART.
// The serial runs at 9600 baud, parity even, 1 stop bit.
UARTPio::UARTPio() : pio(pio0), sm(0)
{
	uint offset = pio_add_program(this->pio, &p1p2_uart_tx_program);
	p1p2_uart_tx_program_init(this->pio, this->sm, offset, UARTPio::PIN_UP, 9600);

	// Prepare for IRQ support
	irq_set_exclusive_handler(PIO0_IRQ_0, pio_irq0_tx_fifo_not_full_handler);
	irq_set_enabled(PIO0_IRQ_0, true);

	// Disable TX FIFO not full interrupt until data is placed in SW FIFO
	pio_set_irq0_source_enabled(this->pio, pis_sm0_tx_fifo_not_full, false);

	gpio_init(UARTPio::PIN_SHUTDOWN);
	gpio_set_dir(UARTPio::PIN_SHUTDOWN, true);
	gpio_put(UARTPio::PIN_SHUTDOWN, false);
}

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
	this->fifo.Clear();
	pio_sm_clear_fifos(this->pio, this->sm);
}

// Send adds data to the HW FIFO in a non blocking way or adds the data to the
// internal FIFO to transmit it as soon as the HW FIFO drains.
void UARTPio::Send(const uint8_t data) {
	if (!pio_sm_is_tx_fifo_full(this->pio, this->sm))
		p1p2_uart_tx(this->pio, this->sm, data);
	else if (this->fifo.Full()) {
		this->error = true;
	} else {
		this->fifo.Push(data);
		pio_set_irq0_source_enabled(this->pio, pis_sm0_tx_fifo_not_full, true);
	}
}

// Pops one byte (if possible) from the SW FIFO and moves it to the HW FIFO.
// If SW FIFO is empty disable interrupt.
void UARTPio::DrainSWFifo(void) {
	if (!this->fifo.Empty()) {
		p1p2_uart_tx(this->pio, this->sm, this->fifo.Front());
		this->fifo.Pop();
	} else {
		// Disable interrupt as SW FIFO is empty
		pio_set_irq0_source_enabled(this->pio, pis_sm0_tx_fifo_not_full, false);
	}
}

// Returns true if the software FIFO has data
bool UARTPio::DataWaiting(void) {
	return !this->fifo.Empty();
}

// Returns true if a buffer overrun was detected
bool UARTPio::Error(void) {
	bool tmp = this->error;
	this->error = false;
	return tmp;
}