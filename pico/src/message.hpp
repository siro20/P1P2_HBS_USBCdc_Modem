#pragma once

#include <inttypes.h>
#define MAX_PACKET_SIZE 32

class Message
{
	public:
		enum STATUS {
			STATUS_OK = 0,
			STATUS_ERR_BUS_COLLISION = 1,
			STATUS_ERR_OVERFLOW = 2,
			STATUS_ERR_PARITY = 3,
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

