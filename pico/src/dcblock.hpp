#pragma once
#include <inttypes.h>

class DCblock
{
	public:
		DCblock(void);

		bool Update(const int32_t in, int32_t *out);

	private:
		int32_t y;
		int32_t x;
};