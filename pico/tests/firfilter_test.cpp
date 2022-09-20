#include <gtest/gtest.h>
#include <math.h>

#include "fir_filter.hpp"

static float SignalRMS(int32_t *signal, size_t len)
{
	float ret = 0;
	for (size_t i = 0; i < len; i++)
		ret += signal[i] * signal[i];
	return sqrt(ret);
}

TEST(FIRfilter, Filter)
{
	FIRFilter f;
	int32_t data[256];
	int32_t out[256];

	for (size_t freq = 1; freq < 128; freq++) {
	for (size_t j=0;j < 256; j++) {
		// Generate Sin stating with lowest frequency first
		data[j] = sin((float)j*2*3.141592/256*(float)freq) * 100;
	}

	for (int i = 0; i < 256; i++)
		f.Update(data[i], &out[i]);

	std::cout << "RMS " << SignalRMS(out, 256) << std::endl;
	}
}

TEST(FIRfilter, Plot)
{
	FIRFilter f;
	int32_t data[256];
	int32_t out[256];

	int j = 0;
	// x32 oversampling
	for (int i = 0; i < 256; i++) {
		data[i] = (i & 16) * 3300;
		if (i & 32)
		data[i] *= -1;
		if (data[i] != 0)
		j++;
		if (j == 14)
		data[i] *= -1;
	}


	for (int i = 0; i < 256; i++) {
		std::cout << data[i] << ", ";
	}
	std::cout <<  std::endl;

	for (int i = 0; i < 256; i++) {
		f.Update(data[i], &out[i]);
		std::cout << out[i] << ", ";
	}
	std::cout <<  std::endl;
}