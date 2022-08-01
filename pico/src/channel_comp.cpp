#include <stdexcept>
#include "channel_comp.hpp"
#include "math.h"
#include <iostream>
using namespace std;

ChannelComp::ChannelComp(void) :
charge(0), resistance(0x800) // Found by empirical testing
{
}

// Update returns false if no new data is available.
// Update returns true if new data has been placed in out.
bool ChannelComp::Update(const int16_t in, int16_t *out) {
	//
	// The AC coupling is required to removed the P1/P2 DC bus voltage.
	// The bias is required to make sure the full signal amplitude can be captured
	// by the ADC.
	// The bias resistors discharge the AC coupling capacitor on "DC" signals as found
	// in the modulated rectangular P1/P2 bus signal.
	// Emulate the capacitor charging and then add the capacitor voltage to the signal.
	//
	this->charge += ((in - this->charge) * resistance) >> 16;

	*out = in + this->charge;
	return true;
}

