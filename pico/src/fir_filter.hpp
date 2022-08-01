#pragma once
#include <inttypes.h>
#include "shiftreg.hpp"

class FIRFilter
{
	public:
		FIRFilter(void);

		bool Update(int16_t in, int16_t *out);

	private:
		ShiftReg<int16_t, 7> reg;
		ShiftReg<int16_t, 7> coeff;
};