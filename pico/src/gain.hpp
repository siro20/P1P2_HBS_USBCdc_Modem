#pragma once
#include <inttypes.h>

class AGC
{
	public:
		AGC(void);

		bool Update(const int32_t in, int32_t *out);

	private:
		int32_t amplitude;
};