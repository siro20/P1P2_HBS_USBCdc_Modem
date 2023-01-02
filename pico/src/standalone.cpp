#include <stdio.h>
#include <pico/platform.h>
#include <pico/stdlib.h>

#include "standalone.hpp"

#define TIMEOUT_IDLE_MS 100
#define TIMEOUT_BUS_SCAN 1000
#define TIMEOUT_OPERATION 60000


StandaloneController::StandaloneController() :
	Answer{}, Address(P1P2_DAIKIN_DEFAULT_EXT_CTRL_ADDR),
	Ready(false), State(IDLE),
	IdleCounterMs(make_timeout_time_ms(TIMEOUT_IDLE_MS)) {

	this->Answer.Data[0] = P1P2_DAIKIN_CMD_ANSWER;
	this->Answer.Data[1] = this->Address;
	this->Answer.Data[2] = P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL;
	for (int i = 3; i < 17; i++) {
			this->Answer.Data[i] = 0;
	}
	// Request 0x35 packet
	this->Answer.Data[3+5-1] = 1;
	// Request 0x38 packet
	this->Answer.Data[3+8-1] = 1;
	this->Answer.Length = 18;

	this->Answer.Data[17] = this->GenCRC(&this->Answer, 17);
}

void StandaloneController::Check(void) {
	switch (this->State) {
	case IDLE:
		// Wait 100 msec to finish possible TxAnswer being sent
		if (time_reached(this->IdleCounterMs)) {
			this->State = BUS_SCAN;
			this->IdleCounterMs = make_timeout_time_ms(TIMEOUT_BUS_SCAN);
		}
	break;
	case BUS_SCAN:
		// Scan the bus for external controller activity
		if (time_reached(this->IdleCounterMs)) {
			this->State = OPERATING;
			this->IdleCounterMs = make_timeout_time_ms(TIMEOUT_OPERATION);
		}
	break;
	case OPERATING:
		// Every 10 minutes stop normal operation for scanning external controller
		if (time_reached(this->IdleCounterMs)) {
			this->State = IDLE;
			this->Ready = false;
			this->IdleCounterMs = make_timeout_time_ms(TIMEOUT_IDLE_MS);
		}
	break;
	}
}

void StandaloneController::Receive(const Message *in) {
	if (in->Length <= 3)
		return;

	switch (this->State) {
	case IDLE:
		return;
	case BUS_SCAN:
		if ((in->Data[0] == P1P2_DAIKIN_CMD_ANSWER) &&
		    (in->Data[1] == this->Address) &&
		    (in->Data[2] == P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL) &&
		    (this->GenCRC(in, in->Length - 1) == in->Data[in->Length - 1])) {
			// Found a conflicting external controller!
			// Switch to next address.
			this->Address++;
			if (this->Address > 0xF1)
				this->Address = P1P2_DAIKIN_DEFAULT_EXT_CTRL_ADDR;
			this->Answer.Data[1] = this->Address;
			this->Answer.Data[17] = this->GenCRC(&this->Answer, 17);
		}
		break;
	case OPERATING:
		if ((in->Data[0] == P1P2_DAIKIN_CMD_REQUEST) &&
		    (in->Data[1] == this->Address) &&
		    (in->Data[2] == P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL) &&
		    (this->GenCRC(in, in->Length - 1) == in->Data[in->Length - 1])) {
			this->Ready = true;
		} else {
			this->Ready = false;
		}
		break;
	}
}

bool StandaloneController::HasTxData(void) {
	return this->Ready;
}

void StandaloneController::TxAnswer(Message *out) {
	*out = this->Answer;
	this->Ready = false;
}

void StandaloneController::BusCollision(void) {
	// On bus collision scan bus for conflicting external controllers
	this->State = IDLE;
	this->IdleCounterMs = make_timeout_time_ms(TIMEOUT_IDLE_MS);
	this->Ready = false;
}

bool StandaloneController::IsTxAnswer(const Message *in) {
	return (in->Data[0] == P1P2_DAIKIN_CMD_ANSWER) &&
		(in->Data[1] == this->Address) &&
		(in->Data[2] == P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL);
}

// Calculates the CRC over Data[0]..Data[len - 1]
// Using uint32_t/size_t for faster assembly code.
uint8_t StandaloneController::GenCRC(const Message *in, size_t len) {
	uint32_t crc = 0;
	for (size_t j = 0; j < len; j++) {
		uint32_t c = in->Data[j];
		for (size_t i = 0; i < 8; i++) {
			if ((crc ^ c) & 0x01) {
				crc = (crc >> 1) ^ 0xd9;
			} else {
				crc = (crc >> 1);
			}
			c >>= 1;
		}
	}
	return crc & 0xff;
}

