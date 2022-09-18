#include "gain.hpp"
#include "defines.hpp"
#include <iostream>
using namespace std;

// Adjust the gain.
AGC::AGC(void) :
amplitude(1000)
{
}

// Update returns false if no new data is available.
// Update returns true if new data has been placed in out.
bool AGC::Update(const int32_t in, int32_t *out) {
	int32_t tmp = in;
	if (in < 0)
		tmp = -in;

	if (tmp > this->amplitude)
		this->amplitude += (tmp - this->amplitude) >> 4;

	if (this->amplitude > BUS_LOW_MV) {
		// Low pass filter, but ignore long idle periods
		this->amplitude --;
	}

	if (this->amplitude > (BUS_HIGH_MV * 2)) {
		*out = in * (BUS_HIGH_MV * 2) / this->amplitude;
	} else {
		*out = in;
	}

	return true;
}

