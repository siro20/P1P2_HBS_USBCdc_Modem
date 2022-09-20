#include <stdexcept>
#include "fir_filter.hpp"
#include "math.h"
#include <iostream>
using namespace std;

// 32x oversampling, lowpass 67200Hz

static const int32_t coefficients[] = {
	-0.071383729317764918 * 0x7fff,
	0.055470186813273974 * 0x7fff,
	0.304697734025869083 * 0x7fff,
	0.436830215005959033 * 0x7fff,
	0.304697734025869138 * 0x7fff,
	0.055470186813273988 * 0x7fff,
	-0.071383729317764918 * 0x7fff,
};

FIRFilter::FIRFilter(void) :
reg(), coeff(coefficients)
{
}

// Update returns false if no new data is available.
// Update returns true if new data has been placed in out.
bool FIRFilter::Update(const int32_t in, int32_t *out) {
	int32_t tmp;
	// Shift in new value
	this->reg.Update(in);

	// Apply the filter
	tmp = ShiftReg<int32_t, 7>::Convolute<int32_t,int32_t, 7, 7>(this->reg, this->coeff);
	*out = tmp >> 15;
	return true;
}

