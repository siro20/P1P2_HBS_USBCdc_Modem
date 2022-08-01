#include <gtest/gtest.h>

#include "resample.hpp"

static uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};
static uint8_t resample_1[] = {0, 2, 4, 6, 8};
static uint8_t resample_2[] = {0, 3, 6,};

TEST(Resample0, TestUint8)
{
	Resample<uint8_t> r(0);
	uint8_t *out = new uint8_t[9];

	size_t j = 0;
	for (size_t i = 0; i < 9; i++) {
		if(r.Update(data[i], &out[j]))
			j++;
	}
	EXPECT_EQ(j, 9);
	for (size_t i = 0; i < 9; i++) {
		EXPECT_EQ(data[i], out[i]);
	}
	delete[] out;
}

TEST(Resample1, TestUint8)
{
	Resample<uint8_t> r(1);
	uint8_t *out = new uint8_t[9];

	size_t j = 0;
	for (size_t i = 0; i < 9; i++) {
		if(r.Update(data[i], &out[j]))
			j++;
	}
	EXPECT_EQ(j, 5);
	for (size_t i = 0; i < 5; i++) {
		EXPECT_EQ(resample_1[i], out[i]);
	}
	delete[] out;
}

TEST(Resample2, TestUint8)
{
	Resample<uint8_t> r(2);
	uint8_t *out = new uint8_t[9];

	size_t j = 0;
	for (size_t i = 0; i < 9; i++) {
		if(r.Update(data[i], &out[j]))
			j++;
	}
	EXPECT_EQ(j, 3);
	for (size_t i = 0; i < 3; i++) {
		EXPECT_EQ(resample_2[i], out[i]);
	}
	delete[] out;
}