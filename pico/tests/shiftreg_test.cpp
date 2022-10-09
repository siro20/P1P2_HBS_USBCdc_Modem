#include <gtest/gtest.h>

#include "shiftreg.hpp"

TEST(Shiftreg, InitialLoadValue)
{
	int16_t buf[3 * 2];
	ShiftReg<int16_t, 3> s(buf);

	EXPECT_EQ(s.At(0), 0);
	EXPECT_EQ(s.At(1), 0);
	EXPECT_EQ(s.At(2), 0);

	int16_t initdata[3] = {1,2,3};

	ShiftReg<int16_t, 3> i(buf, initdata);
	EXPECT_EQ(i.At(0), 1);
	EXPECT_EQ(i.At(1), 2);
	EXPECT_EQ(i.At(2), 3);
}

TEST(Shiftreg, TestShift)
{
	int16_t buf[3 * 2];
	int16_t initdata[3] = {0, 0, 1};
	ShiftReg<int16_t, 3> s(buf, initdata);
	int16_t newdata = 0;

	EXPECT_EQ(s.At(0), 0);
	EXPECT_EQ(s.At(1), 0);
	EXPECT_EQ(s.At(2), 1);

	s.Update(newdata, nullptr);
	EXPECT_EQ(s.At(0), 0);
	EXPECT_EQ(s.At(1), 1);
	EXPECT_EQ(s.At(2), 0);

	s.Update(newdata, nullptr);
	EXPECT_EQ(s.At(0), 1);
	EXPECT_EQ(s.At(1), 0);
	EXPECT_EQ(s.At(2), 0);

	s.Update(newdata, nullptr);
	EXPECT_EQ(s.At(0), 0);
	EXPECT_EQ(s.At(1), 0);
	EXPECT_EQ(s.At(2), 0);
}

TEST(Shiftreg, TestOldestDataAtZero)
{
	int16_t buf[3 * 2];
	int16_t data[3] = {1, 2, 3};
	ShiftReg<int16_t, 3> s(buf);

	for (int i = 0; i < 3; i++) {
		s.Update(data[i], nullptr);
	}

	EXPECT_EQ(s.At(0), 1);
	EXPECT_EQ(s.At(1), 2);
	EXPECT_EQ(s.At(2), 3);
}

TEST(Shiftreg, TestBigShiftRef)
{
	int16_t buf[256 * 2];
	ShiftReg<int16_t, 256> s(buf);

	for (int i = 0; i < 128; i++) {
		s.Update(0, nullptr);
	}
}

TEST(Shiftreg, Convolute)
{
	int16_t buf_a[3 * 2];
	int16_t initdata[3] = {0, 0, 1};
	ShiftReg<int16_t, 3> s(buf_a, initdata);
	int16_t result;
	{
		int16_t buf_b[3 * 2];
		int16_t initdata2[3] = {0, 0, 1};
		ShiftReg<int16_t, 3> t(buf_b, initdata2);
		result = ShiftReg<int16_t, 3>::Convolute<int16_t,int16_t, 3, 3>(s, t);
		EXPECT_EQ(result, 1);
	}
	{
		int16_t buf_b[3 * 2];
		int16_t initdata2[3] = {0, 1, 0};
		ShiftReg<int16_t, 3> t(buf_b, initdata2);
		result = ShiftReg<int16_t, 3>::Convolute<int16_t,int16_t, 3, 3>(s, t);
		EXPECT_EQ(result, 0);
	}
	{
		int16_t buf_b[3 * 2];
		int16_t initdata2[3] = {1, 0, 0};
		ShiftReg<int16_t, 3> t(buf_b, initdata2);
		result = ShiftReg<int16_t, 3>::Convolute<int16_t,int16_t, 3, 3>(s, t);
		EXPECT_EQ(result, 0);
	}
	{
		int16_t buf_b[3 * 2];
		int16_t initdata2[3] = {1, 1, -1};
		ShiftReg<int16_t, 3> t(buf_b, initdata2);
		result = ShiftReg<int16_t, 3>::Convolute<int16_t,int16_t, 3, 3>(s, t);
		EXPECT_EQ(result, -1);
	}
}