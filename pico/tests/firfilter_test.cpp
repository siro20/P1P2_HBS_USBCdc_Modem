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
	int32_t buf_a[7 * 2];
	int32_t buf_b[7 * 2];
	const int steps[] = {1, 16, 32, 48, 64, 92, 127};

	FIRFilter f(buf_a, buf_b);

	for (size_t j = 0; j < 7; j++) {
		const size_t freq = steps[j];
		int32_t data[256];
		int32_t out[256];

		for (size_t j=0;j < 256; j++) {
			// Generate Sin stating with lowest frequency first
			data[j] = sin((float)j*2*3.141592/256*(float)freq) * 100;
		}

		for (int i = 0; i < 256; i++)
			f.Update(data[i], &out[i]);

		switch (freq) {
			case 1: EXPECT_GT(SignalRMS(out, 256), 1100); break;
			case 16: EXPECT_GT(SignalRMS(out, 256), 1100); break;
			case 32: EXPECT_GT(SignalRMS(out, 256), 1000); break;
			case 48: EXPECT_LT(SignalRMS(out, 256), 850); break;
			case 64: EXPECT_LT(SignalRMS(out, 256), 400); break;
			case 92: EXPECT_LT(SignalRMS(out, 256), 150); break;
			case 127: EXPECT_LT(SignalRMS(out, 256), 100); break;
		}
	}

}
