#include <stdio.h>
#include <pico.h>
#include <pico/stdlib.h>

#include "standalone.hpp"

#define TIMEOUT_IDLE_MS 100
#define TIMEOUT_BUS_SCAN 2000
#define TIMEOUT_OPERATION 600000

// Response to specific packets on the bus to
// emulate an 'external controller'.

StandaloneController::StandaloneController() :
	Answer{}, Non3xhPacket{}, Address(P1P2_DAIKIN_DEFAULT_EXT_CTRL_ADDR),
	Ready(false), State(IDLE),
	IdleCounterMs(make_timeout_time_ms(TIMEOUT_IDLE_MS)),
	ExtCtrlPacketsTodo(0) {
}

// Periodic state machine function
// Must be regulary called.
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

// Returns true when 3xh packets needs to be exchanged (bus is busy)
bool StandaloneController::ExtCtrlPhase(void) {
	return this->ExtCtrlPacketsTodo > 0;
}

// Returns true when the last 3xh packets is exchanged (bus is busy)
bool StandaloneController::ExtCtrlPhaseEndsNow(void) {
	return this->ExtCtrlPacketsTodo == 1;
}

// Update bus busy status
void __attribute__((optimize("no-unroll-loops"))) StandaloneController::UpdateExtCtrlPhase(const Message *in)  {
	size_t packets_todo;
	if (in->Length <= 3)
		return;

	if (in->Data[1] != this->Address) {
		// Only accept packets for the external controller address.
		return;
	}

	switch (in->Data[2]) {
		case P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL:
			packets_todo = 0;
			// Count packets to be transmitted
			for (int i = 3; i < in->Length - 1; i++)
				packets_todo += in->Data[i];

			packets_todo *= 2;
			if (in->Data[0] == P1P2_DAIKIN_CMD_REQUEST) {
				packets_todo++;
			}
			this->ExtCtrlPacketsTodo = packets_todo;
			break;
		case P1P2_DAIKIN_TYPE_STATUS_EXT_CTRL...P1P2_DAIKIN_TYPE_EXT_LAST:
			if (this->ExtCtrlPacketsTodo > 0)
				this->ExtCtrlPacketsTodo --;
			break;
		default:
			return;
	}
}

// Receive a paket and decide if it's valid
// and needs to be handled.
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
		}
		break;
	case OPERATING:
		this->UpdateExtCtrlPhase(in);
		if (this->NeedToHandlePacket(in) &&
		    this->GenCRC(in, in->Length - 1) == in->Data[in->Length - 1]) {
			this->GenerateAnswer(in);
		} else {
			this->Ready = false;
		}
		break;
	}
}

bool StandaloneController::NeedToHandlePacket(const Message *in) {
	if (in->Data[0] != P1P2_DAIKIN_CMD_REQUEST) {
		// External controller only answers requests.
		// Ignore answers.
		return false;
	}
	if (in->Data[1] != this->Address) {
		// Only accept packets for the external controller address.
		return false;
	}

	uint8_t type = in->Data[2];
	switch (type) {
	case P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL:
		// Respond here to enable communication using packets 00f031..00f03f
		return true;
	case P1P2_DAIKIN_TYPE_STATUS_EXT_CTRL:
		// Do pretend to be a LAN adapter (even though this may trigger "data not in sync" upon restart?)
		// If we don't set address, installer mode in main thermostat may become inaccessible
		return true;
	case P1P2_DAIKIN_TYPE_PARAM_EXT_CTRL...P1P2_DAIKIN_TYPE_EXT_LAST:
		{
			// Always answer to reduce remote waiting 150msec for an answer
			// When no cached packet is available generate NULL packet
			return true;
		}
	}

	return false;
}

// Received a valid paket that needs to be handled
// Generates a response message.
void StandaloneController::GenerateAnswer(const Message *in) {
	uint8_t type = in->Data[2];

	this->Ready = false;

	switch (type) {
	case P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL:
		this->Answer.Data[0] = P1P2_DAIKIN_CMD_ANSWER;
		this->Answer.Data[1] = this->Address;
		this->Answer.Data[2] = P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL;
		for (int i = 3; i < 17; i++) {
			this->Answer.Data[i] = in->Data[i];

			// Request packet 3xh to be handled in the current cycle
			if (i >= 4) {
				// Returning 0 here doesn't prevent the other side from sending the packet.
				// Thus only request to handle the packet when the remote doesn't want
				// to send a packet yet.
				if ((this->Packet3xh[i - 4].Length > 3) && this->Answer.Data[i] == 0)
					this->Answer.Data[i] = 1;
			}
		}

		this->Answer.Length = 18;
	break;

	case P1P2_DAIKIN_TYPE_STATUS_EXT_CTRL:
		this->Answer.Data[0] = P1P2_DAIKIN_CMD_ANSWER;
		this->Answer.Data[1] = this->Address;
		this->Answer.Data[2] = P1P2_DAIKIN_TYPE_STATUS_EXT_CTRL;
		for (int i = 3; i < 15; i++) {
			this->Answer.Data[i] = in->Data[i];
		}
		this->Answer.Data[7] = 0xB4; // LAN adapter ID in 0x31 payload byte 7
		this->Answer.Data[8] = 0x10; // LAN adapter ID in 0x31 payload byte 8
		this->Answer.Length = 16;
	break;

	// Check if cached response needs to be transmitted
	case P1P2_DAIKIN_TYPE_PARAM_EXT_CTRL...P1P2_DAIKIN_TYPE_EXT_LAST:
		{
			uint8_t idx = type - P1P2_DAIKIN_TYPE_PARAM_EXT_CTRL;
			this->Answer.Data[0] = P1P2_DAIKIN_CMD_ANSWER;
			this->Answer.Data[1] = this->Address;
			this->Answer.Data[2] = type;

			if (this->Packet3xh[idx].Length) {
				// Copy cached payload
				for (int i = 3; i < this->Packet3xh[idx].Length - 1; i++) {
					this->Answer.Data[i] = this->Packet3xh[idx].Data[i];
				}
				this->Answer.Length = this->Packet3xh[idx].Length;

				// Mark cached packet as transmitted
				this->Packet3xh[idx].Length = 0;
			} else if (this->Non3xhPacket.Length) {
				// Protocol violation! Send a 'wrong' packet here!
				// The remote waits about 180 msec for a correct response.
				//
				// Send a non 3xh packet instead of correct answer to avoid bus collision.
				// Tests showed that the P1P2 control unit does not monitor the bus and
				// starts transmitting after a fixed delay, overwriting a currently transmitted
				// packet. As it waits here 180msec, this gives the oppertunity to transmit and
				// receive custom packets here.
				//

				// Copy cached payload
				for (int i = 0; i < this->Non3xhPacket.Length - 1; i++) {
					this->Answer.Data[i] = this->Non3xhPacket.Data[i];
				}
				this->Answer.Length = this->Non3xhPacket.Length;
	
				this->Non3xhPacket.Length = 0;
			} else {
				// No cached packet, respond with NULL data packet (all bytes 0xff).
				// This prevents a timeout on the remote waiting for an answer:
				//   The remote waits about 150msec, thus there's a 180msec gap between two
				//   packets when no answer is being transmitted.
				//
				// With this code the gap between 3xh packets is 120msec.
				int len;
				if (type == 0x35 || type == 0x3a || type == 0x38 || type == 0x39 || type == 0x3d)
					len = 22;
				else if (type == 0x36 || type == 0x3b || type == 0x37 || type == 0x3c)
					len = 24;
				else {
					return;
				}
				// Generate NULL answer
				for (int i = 3; i < len - 1; i++)
					this->Answer.Data[i] = 0xff;
				this->Answer.Length = len;
			}

			break;
		}

	default:
		return;
	}

	// Fix CRC
	this->Answer.Data[this->Answer.Length - 1] =
			this->GenCRC(&this->Answer, this->Answer.Length - 1);
	this->Ready = true;
}

// Returns true when TxAnswer should be transmitted.
// Only true as long as TxAnswer() has not been called.
// Only true till another packet is received, aka. Receive() is called
bool StandaloneController::HasTxData(void) {
	return this->Ready;
}

// Returns true when a non 3xh packet is waiting for transmission
bool StandaloneController::Non3xhPacketWaitForTransmission(void) {
	return this->Non3xhPacket.Length > 0;
}

// Cache a message and transmit it on the next free slot
bool StandaloneController::CacheTxMessage(Message& in) {
	if (in.Length <= 3)
		return false;

	if (in.Data[2] < P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL ||
		in.Data[2] > P1P2_DAIKIN_TYPE_EXT_LAST) {
		this->Non3xhPacket = in;
		return true;
	}

	if (in.Data[0] != P1P2_DAIKIN_CMD_ANSWER ||
		in.Data[2] < P1P2_DAIKIN_TYPE_PARAM_EXT_CTRL ||
		in.Data[2] > P1P2_DAIKIN_TYPE_EXT_LAST)
		return false;

	uint8_t idx = in.Data[2] - P1P2_DAIKIN_TYPE_PARAM_EXT_CTRL;

	// Still have old packet in cache, abort...
	if (this->Packet3xh[idx].Length > 3)
		return false;
	
	this->Packet3xh[idx] = in;
	return true;
}

// The Msg to be transmitted.
// Calling this functions resets HasTxData()
void StandaloneController::TxAnswer(Message *out) {
	out->Length = this->Answer.Length;
	for (int i = 0; i < out->Length; i++) {
		out->Data[i] = this->Answer.Data[i];
	}
	this->Ready = false;
}

void StandaloneController::BusCollision(void) {
	// On bus collision scan bus for conflicting external controllers
	this->State = IDLE;
	this->IdleCounterMs = make_timeout_time_ms(TIMEOUT_IDLE_MS);
	this->Ready = false;
}

// Returns true if packet is generated by this instance.
bool StandaloneController::IsTxAnswer(const Message *in) {
	if (in->Data[0] != P1P2_DAIKIN_CMD_ANSWER)
		return false;

	if (in->Data[1] == this->Address)
		return false;

	return (in->Data[2] >= P1P2_DAIKIN_TYPE_SENSE_EXT_CTRL) &&
	       (in->Data[2] <= P1P2_DAIKIN_TYPE_EXT_LAST);
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

