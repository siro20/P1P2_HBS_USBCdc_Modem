#include <inttypes.h>
#include "host_uart.hpp"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include <iostream>

static void on_uart_irq() {
	HostUART& u = HostUART::getInstance();
	u.Check();
}

HostUART::HostUART() :
	LineReceiver(), time(0), error(false), tx_fifo(), rx_fifo()
{
	// Set UART flow control CTS/RTS, we don't want these, so turn them off
	uart_set_hw_flow(uart0, false, false);

	// And set up and enable the interrupt handlers
	irq_set_exclusive_handler(UART0_IRQ, on_uart_irq);
	irq_set_enabled(UART0_IRQ, true);

	// Now enable the UART to send interrupts
	uart_set_irq_enables(uart0, true, true);
}

HostUART::~HostUART() 
{
	uart_set_irq_enables(uart0, false, false);

	irq_set_enabled(UART0_IRQ, false);

	irq_remove_handler(UART0_IRQ, on_uart_irq);
}

// Receiving returns true as long as data is being received
void HostUART::CheckRXFIFO(void) {
	while (uart_is_readable(uart0)) {
		uint8_t c = uart_getc(uart0);

		if (!this->Full())
			this->Push(c);
	}
}

void HostUART::CheckTXFIFO(void) {
	while (uart_is_writable(uart0) && !this->tx_fifo.Empty()) {
		uart_putc(uart0, this->tx_fifo.Front());
		this->tx_fifo.Pop();
	}
}

void HostUART::Check(void) {
	this->CheckTXFIFO();
	this->CheckRXFIFO();
}

bool HostUART::HasData(void) {
	return !this->rx_fifo.Empty();
}

void HostUART::OnLineReceived(uint8_t *line) {
	Message m((char *)line);

	// Update internal time based on host time
	if (m.Time > 0)
		this->SetTime(m.Time);

	// TODO: Status

	this->rx_fifo.Push(m);
}

void HostUART::UpdateAndSend(Message& m) {
	m.Time = this->GetTime();
	if (this->error) {
		m.Status |= Message::STATUS_ERR_OVERFLOW;
		this->error = false;
	}
	this->Send(m);
}

void HostUART::Send(Message& m) {
	const char *line = m.c_str();
	while (line[0]) {
		if (!this->tx_fifo.Full()) {
			this->tx_fifo.Push(line[0]);
			this->CheckTXFIFO();
		} else {
			this->error = true;
		}
		line++;
	}
	if (!this->tx_fifo.Full()) {
		this->tx_fifo.Push('\n');
		this->CheckTXFIFO();
	} else {
		this->error = true;
	}
}

void HostUART::SetTime(uint64_t t) {
	this->time = t - to_ms_since_boot(get_absolute_time());
}

uint64_t HostUART::GetTime(void) {
	return this->time + to_ms_since_boot(get_absolute_time());
}
