#include <gtest/gtest.h>
#include <math.h>

#include "uart_bit_detect_fast.hpp"
#include "level_detect.hpp"

#define LVL_HIGH 1400
#define LVL_LOW 600

TEST(UartBitDetect, TestRealWorldHigh)
{
	int16_t buf_a[16 * 2];
	int16_t buf_b[16 * 2];
	UARTBit<int16_t, 16> b(buf_a, buf_b, LVL_HIGH, LVL_LOW, 0xe0);
	Level<int16_t> l;

	int16_t data[16];
	int16_t out[16];

	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 9; i++) {
		data[i] = LVL_HIGH;
	}
	for (size_t i = 9; i < 17; i++) {
		data[i] = LVL_LOW - 1;
	}
	
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}

	EXPECT_EQ(out[15], 6206);

	for (size_t i = 9; i < 17; i++) {
		data[i] = -LVL_LOW + 1;
	}
	
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}

	EXPECT_EQ(out[15], 6206);
}

TEST(UartBitDetect, TestRealWorldLow)
{
	int16_t buf_a[16 * 2];
	int16_t buf_b[16 * 2];
	UARTBit<int16_t, 16> b(buf_a, buf_b, LVL_HIGH, LVL_LOW, 0xe0);
	Level<int16_t> l;

	int16_t data[16];
	int16_t out[16];

	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 9; i++) {
		data[i] = -LVL_HIGH;
	}
	for (size_t i = 9; i < 17; i++) {
		data[i] = -LVL_LOW + 1;
	}
	
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}

	EXPECT_EQ(out[15], -6206);

	for (size_t i = 9; i < 17; i++) {
		data[i] = LVL_LOW - 1;
	}
	
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}

	EXPECT_EQ(out[15], -6206);
}

TEST(UartBitDetect, High)
{
	int16_t buf_a[16 * 2];
	int16_t buf_b[16 * 2];
	UARTBit<int16_t, 16> b(buf_a, buf_b, 1, 0, 0xff);
	int16_t data[16];
	int16_t out[16];

	// Test high phase 8 samples long
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 9; i++) {
		data[i] = 1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}

	EXPECT_EQ(out[15], 7);

	// Test high phase 7 samples long
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 8; i++) {
		data[i] = 1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}

	EXPECT_EQ(out[15], 7);

	// Test high phase 6 samples long
	// Detects as line idle as bit error is 0xff == 0%
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 7; i++) {
		data[i] = 1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}

	EXPECT_EQ(out[15], 0);
}

TEST(UartBitDetect, Low)
{
	int16_t buf_a[16 * 2];
	int16_t buf_b[16 * 2];
	UARTBit<int16_t, 16> b(buf_a, buf_b, 1, 0, 0xff);
	int16_t data[16];
	int16_t out[16];

	// Test low phase 8 samples long
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 9; i++) {
		data[i] = -1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}

	EXPECT_EQ(out[15], -7);

	// Test low phase 7 samples long
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 8; i++) {
		data[i] = -1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}

	// Test low phase 6 samples long
	// Detects as line idle as bit error is 0xff == 0%
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 7; i++) {
		data[i] = -1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}
	EXPECT_EQ(out[15], 0);
}


TEST(UartBitDetect, BitError)
{
	int16_t buf_a[16 * 2];
	int16_t buf_b[16 * 2];
	UARTBit<int16_t, 16> b(buf_a, buf_b, 1, 0, 0x80);
	int16_t data[16];
	int16_t out[16];

	// Test low phase 6 samples long
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 7; i++) {
		data[i] = -1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}
	EXPECT_EQ(out[15], -6);

	// Test low phase 5 samples long
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 6; i++) {
		data[i] = -1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}
	EXPECT_EQ(out[15], -5);

	// Test low phase 4 samples long
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 5; i++) {
		data[i] = -1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}
	EXPECT_EQ(out[15], -4);
	
	// Test low phase 3 samples long
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 4; i++) {
		data[i] = -1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}
	EXPECT_EQ(out[15], -3);

	// Test low phase 2 samples long
	// Detects as line idle as bit error is 0x80 == 50%
	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 1; i < 3; i++) {
		data[i] = -1;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}
	EXPECT_EQ(out[15], 0);
}

TEST(UartBitDetect, DCLevel)
{
	int16_t buf_a[16 * 2];
	int16_t buf_b[16 * 2];
	UARTBit<int16_t, 16> b(buf_a, buf_b, LVL_HIGH, LVL_LOW, 0xe0);
	int16_t data[16];
	int16_t out[16];

	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 0; i < 16; i++) {
		data[i] = LVL_HIGH;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}
	EXPECT_EQ(out[14], 0);

	memset(data, 0, sizeof(*data) * 16);
	for (size_t i = 0; i < 16; i++) {
		data[i] = -LVL_HIGH;
	}
	for (size_t i = 0; i < 16; i++) {
		b.Update(data[i], &out[i]);
	}
	EXPECT_EQ(out[14], 0);
}