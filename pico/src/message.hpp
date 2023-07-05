#pragma once

#include <inttypes.h>
#define MAX_PACKET_SIZE 32

class Message
{
	public:
		enum STATUS {
			// Received a packet. No parity error or framing error.
			STATUS_OK = 0,
			// Did not RX the exact same data that was TXed
			STATUS_ERR_BUS_COLLISION = 1,
			// A buffer overflowed.
			STATUS_ERR_OVERFLOW = 2,
			// Parity error.
			STATUS_ERR_PARITY = 3,
			// Did not detect line idle within timeout. Just receiving noise?
			STATUS_ERR_NO_FRAMING = 4,
			// Internal error
			STATUS_INTERNAL_ERROR = 5,
		};
	
		Message();
		Message(uint64_t time, uint32_t status, uint8_t *data, uint8_t length);
		Message(char *line);

		void Append(uint8_t data);
		void Clear(void);
		bool Overflow(void);

		const char* c_str();

		uint32_t Status;
		uint64_t Time;

		uint8_t Data[MAX_PACKET_SIZE];
		uint8_t Length;
};

