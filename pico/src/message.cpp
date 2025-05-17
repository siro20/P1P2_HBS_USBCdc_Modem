
#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include "message.hpp"
#include <iostream>

Message::Message() :
	Status(0), Length(0)
{
}

Message::Message(uint32_t status, uint8_t *data, uint8_t length) :
	Status(status), Length(length < sizeof(this->Data) ? length : sizeof(this->Data))
{
	memcpy(this->Data, data, this->Length);
}

// c_str returns the Message as c string representation
// The returned data is valid until c_str is called again.
const char *Message::c_str(void)
{
	static char line[128];
	char c;
	size_t off;

	for (size_t i = 0; i < this->Length; i++)
		snprintf(&line[i*2], sizeof(line) - (i*2), "%02x", this->Data[i]);

	if (this->Status) {
		off = 2 * this->Length;

		if (this->Status == STATUS_ERR_BUS_COLLISION)
			c = 'C';
		else if (this->Status == STATUS_ERR_OVERFLOW)
			c = 'O';
		else if (this->Status == STATUS_ERR_PARITY)
			c = 'P';
		else if (this->Status == STATUS_ERR_NO_FRAMING)
			c = 'F';
		else
			c = ' ';

		snprintf(&line[off], sizeof(line) - off, " # %02x %c", this->Status, c);
	}

	return line;
}

// Decodes the string to message format
// The overal format is
// time (decimal): status bits (uint32 hex): data (uint8 hex)
Message::Message(char *line)
{
	char *data_ptr;
	int field;
	char decoded;

	this->Status = 0;
	this->Length = 0;

	data_ptr = line;

	field = 0;
	while (data_ptr[0]) {
		if (data_ptr[0] == ' ') {
			data_ptr++;
			continue;
		}

		if (data_ptr[0] == '#' || data_ptr[0] == ';')
			break;

		decoded = -1;
		if (data_ptr[0] >= 'a' && data_ptr[0] <= 'f')
			decoded = data_ptr[0] - 'a' + 0xa;
		else if (data_ptr[0] >= 'A' && data_ptr[0] <= 'F')
			decoded = data_ptr[0] - 'A' + 0xa;
		else if (data_ptr[0] >= '0' && data_ptr[0] <= '9')
			decoded = data_ptr[0] - '0';

		if (decoded != -1) {
			if (field == 0) {
				field = 1;
				this->Data[this->Length] = decoded << 4;
			} else {
				this->Data[this->Length] |= decoded;
				field = 0;
				this->Length++;
			}
		}
		data_ptr++;
	};
}

void Message::Append(uint8_t data)
{
	if (this->Length < sizeof(this->Data)) {
		this->Data[this->Length++] = data;
	}
}

void Message::Clear(void)
{
	this->Length = 0;
	this->Status = 0;
}

bool Message::Overflow(void)
{
	return this->Length == sizeof(this->Data);
}