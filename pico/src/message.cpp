
#include <string.h>
#include <stdlib.h>
#include <cstdio>
#include "message.hpp"
#include <iostream>

Message::Message() :
	Status(0), Time(0), Length(0)
{
}

Message::Message(uint64_t time, uint32_t status, uint8_t *data, uint8_t length) :
	Status(status), Time(time), Length(length < sizeof(this->Data) ? length : sizeof(this->Data))
{
	memcpy(this->Data, data, this->Length);
}

// c_str returns the Message as c string representation
// The returned data is valid until c_str is called again.
const char *Message::c_str(void)
{
	static char line[128];
	char tmp[3];

	snprintf(line, sizeof(line), "%016u:%08x:", this->Time, this->Status);
	for (size_t i = 0; i < this->Length; i++) {
		snprintf(tmp, sizeof(tmp), "%02x", this->Data[i]);
		strcat(line, tmp);
	}

	return line;
}

// Decodes the string to message format
// The overal format is
// time (decimal): status bits (uint32 hex): data (uint8 hex)
Message::Message(char *line)
{
	char *ptr = line;
	char *time_ptr, *status_ptr, *data_ptr;
	int field = 0;
	char decoded;

	this->Time = 0;
	this->Status = 0;
	this->Length = 0;

	time_ptr = NULL;
	status_ptr = NULL;
	data_ptr = line;
	while (ptr[0]) {
		switch (ptr[0]) {
		case ';':
		case '/':
		case '#':
			break;

		case ':': // field delimiter
			ptr[0] = 0;
			if (field == 0) {
				time_ptr = line;
				status_ptr = &ptr[1];
			} else if (field == 1) {
				data_ptr = &ptr[1];
			}
			field++;
			break;
		}

		ptr++;
	};

	if (time_ptr && strlen(time_ptr) > 0) {
		this->Time = atoll(time_ptr);
	}
	if (status_ptr && strlen(status_ptr) > 0) {
		this->Status = strtol(status_ptr, NULL, 16);
	}

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

