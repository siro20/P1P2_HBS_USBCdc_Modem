#pragma once
#include <inttypes.h>
#include "shiftreg.hpp"

class FIRFilter
{
	public:
		FIRFilter(int32_t buffer_a[7 * 2], int32_t buffer_b[7 * 2]);

		bool Update(const int32_t in, int32_t *out);

	private:
		ShiftReg<int32_t, 7> reg;
		ShiftReg<int32_t, 7> coeff;
};