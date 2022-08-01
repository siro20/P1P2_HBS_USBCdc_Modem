#pragma once
#include <inttypes.h>

class AGC
{
	public:
		AGC(void);

		bool Update(int16_t in, int16_t *out);

	private:
		int32_t amplitude;
};