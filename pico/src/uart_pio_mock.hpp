#pragma once
#include "message.hpp"

// High level abstraction of P1P2 UART transmit functions
class UARTPio
{
	public:
	UARTPio(UARTPio const&) = delete;
	void operator=(UARTPio const&) = delete;

	UARTPio(void) : error(false) {}

	bool Transmitting(void) {return false;}
	void Send(const Message& m) {}
	bool DataWaiting(void) {return false;}
	void EnableShutdown(bool state) {}
	void ClearFifo(void) {}
	bool Error(void) {return this->error;}

	void MokSetError(bool err) {this->error = err;}

	private:
		bool error;
};
