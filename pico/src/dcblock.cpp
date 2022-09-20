#include "dcblock.hpp"
#include "defines.hpp"
#include <iostream>
using namespace std;

// Adjust the gain.
DCblock::DCblock(void) : y(0), x(0)
{
}

// Update returns false if no new data is available.
// Update returns true if new data has been placed in out.
bool DCblock::Update(const int32_t in, int32_t *out) {
	int32_t tmp, xn, yn;

	xn = (in << 8);

	tmp = xn - this->x + (((uint16_t)(0.995 * 256) * this->y) >> 8);

	this->x = xn;
	this->y = tmp;

	*out = tmp >> 8;

	return true;
}

