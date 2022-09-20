#include <gtest/gtest.h>

#include "dcblock.hpp"

TEST(DCBlock, Test)
{
	DCblock b;
	int32_t out;
	int32_t testdata[1024];

	for (size_t i = 0; i < sizeof(testdata)/sizeof(testdata[0]); i++) {
		if (i > 300 && i < 310) {
			testdata[i] = 100;
		} else {
			testdata[i] = 10;
		}
	}
	for (size_t i = 0; i < sizeof(testdata)/sizeof(testdata[0]); i++) {
		b.Update(testdata[i], &out);
		
		if (i < 300)
			continue;

		if (i > 300 && i < 310) {
			EXPECT_LT(out, 92);
			EXPECT_GT(out, 84);
		} else {
			EXPECT_LT(out, 1);
			EXPECT_GT(out, -7);
		} 
	}
}
