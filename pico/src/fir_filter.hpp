#pragma once
#include <inttypes.h>
#include "shiftreg.hpp"

class FIRFilter
{
	public:
		FIRFilter(void);

		bool Update(const int32_t in, int32_t *out);

	private:
		ShiftReg<int32_t, 7> reg;
		ShiftReg<int32_t, 7> coeff;
};