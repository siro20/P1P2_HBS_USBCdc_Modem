#pragma once
#include "inttypes.h"

template <class T>
class Resample
{
	public:
	// Drops samples and thus does uniform resampling
	// n gives the number of samples to drop after a non dropped sample.
	// n == 0, no samples are dropped
	// n == 1, 1/2 sample rate
	// n == 2, 1/3 sample rate ...
	Resample(const size_t samples) : counter(0), n(samples)
	{
	}

	// Update returns false if no new data is available.
	// Update returns true if new data has been placed in out.
	bool Update(const T in, T *out) {
		bool ret = false;
		if (counter == 0) {
			*out = in;
			ret = true;
		}
		counter++;
		if (counter > this->n) {
			counter = 0;
		}
		return ret;
	}

	private:
		size_t counter;
		size_t n;
};
