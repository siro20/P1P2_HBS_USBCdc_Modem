#include <gtest/gtest.h>
#include <math.h>

#include "uart_bit_detect_fast.hpp"

TEST(UartBitDetect, TestRealWorld)
{
	UARTBit<int16_t, 16> b;
	int16_t data[256];
	int16_t out_h[256];
	int16_t out_l[256];

	memset(data, 0, sizeof(*data) * 256);
	for (size_t i = 16; i < 24; i++) {
		data[i] = 3300;
	}
	for (size_t i = 64; i < 72; i++) {
		data[i] = -3300;
	}
	for (size_t i = 0; i < 256; i++)
		b.Update(data[i], &out_h[i], &out_l[i]);

	EXPECT_GT(out_h[30], 20000);
	EXPECT_EQ(out_h[0], 0);
	EXPECT_EQ(out_h[24+16], 0);
	EXPECT_LT(out_l[30], -20000);

	EXPECT_GT(out_l[78], 20000);
	EXPECT_EQ(out_l[0], 0);
	EXPECT_EQ(out_l[72+16], 0);
	EXPECT_LT(out_h[78], -20000);
}

TEST(UartBitDetect, High)
{
	UARTBit<int16_t, 16> b;
	int16_t data[16];
	int16_t out_h[16];
	int16_t out_l[16];

	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 0; i < 8; i++) {
		data[i] = 1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out_h[i], &out_l[i]);
	}

	EXPECT_EQ(out_h[14], 8);
	EXPECT_EQ(out_l[14], -8);
}

TEST(UartBitDetect, Low)
{
	UARTBit<int16_t, 16> b;
	int16_t data[16];
	int16_t out_h[16];
	int16_t out_l[16];

	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 0; i < 8; i++) {
		data[i] = -1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out_h[i], &out_l[i]);
	}

	EXPECT_EQ(out_h[14], -8);
	EXPECT_EQ(out_l[14], 8);
}
