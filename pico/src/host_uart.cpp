#include <inttypes.h>
#include "host_uart.hpp"
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/bootrom.h"
#include <iostream>

static void on_uart_irq() {
	HostUART& u = HostUART::getInstance();
	u.CheckTXFIFO();
	u.CheckRXFIFO();
}

HostUART::HostUART() :
	error(false), tx_fifo(), rx_fifo(), rx_msgs_ext_ctrl(), rx_msgs_generic()
{
	uart_set_baudrate(uart0, 115200);

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
	int c;
	while ((c = getchar_timeout_us(0)) >= 0) {
		if (this->rx_fifo.Full()) {
			// Should never happen
			this->rx_fifo.Clear();
		}
		if (c == '\r' || c == '\n') {
			if (!this->rx_fifo.Empty()) {
				this->rx_fifo.Push(0);
				this->OnLineReceived(this->rx_fifo.Data());
				this->rx_fifo.Clear();
			}
		} else {
			this->rx_fifo.Push(c);
		}
	}
}

void HostUART::CheckTXFIFO(void) {
	while (uart_is_writable(uart0) && !this->tx_fifo.Empty()) {
		uint8_t c;
		if (this->tx_fifo.Pop(&c))
			putchar_raw(c);
	}
}


void HostUART::Check(void) {
	this->CheckTXFIFO();
	this->CheckRXFIFO();
}

bool HostUART::HasDataExtController(void) {
	return !this->rx_msgs_ext_ctrl.Empty();
}

bool HostUART::HasDataGeneric(void) {
	return !this->rx_msgs_generic.Empty();
}

Message HostUART::PopExtController(void) {
	Message m;
	if (!this->rx_msgs_ext_ctrl.Empty()) {
		this->rx_msgs_ext_ctrl.Pop(&m);
	}

	return m;
}

Message HostUART::PopGeneric(void) {
	Message m;
	if (!this->rx_msgs_generic.Empty()) {
		this->rx_msgs_generic.Pop(&m);
	}

	return m;
}

void HostUART::OnLineReceived(char *line) {
	if (line[0] == ';' && line[1] == '!' && line[2] == 'B' && line[3] == 'L' &&
	    line[4] == 'D' && line[5] == '!' && line[6] == ';') {
		reset_usb_boot(0,0);
		return;
	}
	if (line[0] == 0 || line[0] == '#' || line[0] == ';') {
		return;
	}

	Message m(line);

	if (m.Length > 3 && m.Data[0] == 0x40 && m.Data[1] == 0xf0 && (m.Data[2] & 0xF0) == 0x30)
		this->rx_msgs_ext_ctrl.Push(m);
	else if (m.Length > 3)
		this->rx_msgs_generic.Push(m);
}

void HostUART::UpdateAndSend(Message& m) {
	if (this->error) {
		m.Status |= Message::STATUS_ERR_OVERFLOW;
		this->error = false;
	}
	this->Send(m);
}

void HostUART::Send(Message& m) {
	uint32_t save;

	const char *line = m.c_str();
	while (line[0]) {
		if (!this->tx_fifo.Full()) {
			this->tx_fifo.Push(line[0]);
		} else {
			this->error = true;
			return;
		}
		line++;
	}
	if (!this->tx_fifo.Full()) {
		this->tx_fifo.Push('\r');
	} else {
		this->error = true;
	}
	if (!this->tx_fifo.Full()) {
		this->tx_fifo.Push('\n');
	} else {
		this->error = true;
	}

	save = save_and_disable_interrupts();
	this->CheckTXFIFO();
	restore_interrupts(save);
}
