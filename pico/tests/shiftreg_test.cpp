#include <gtest/gtest.h>

#include "shiftreg.hpp"

TEST(Shiftreg, InitialLoadValue)
{
	ShiftReg<int16_t, 3> s;

	EXPECT_EQ(s.At(0), 0);
	EXPECT_EQ(s.At(1), 0);
	EXPECT_EQ(s.At(2), 0);

	int16_t initdata[3] = {1,2,3};

	ShiftReg<int16_t, 3> i(initdata);
	EXPECT_EQ(i.At(0), 1);
	EXPECT_EQ(i.At(1), 2);
	EXPECT_EQ(i.At(2), 3);
}

TEST(Shiftreg, TestShift)
{
	int16_t initdata[3] = {0, 0, 1};
	ShiftReg<int16_t, 3> s(initdata);
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
	int16_t data[3] = {1, 2, 3};
	ShiftReg<int16_t, 3> s;

	for (int i = 0; i < 3; i++) {
		s.Update(data[i], nullptr);
	}

	EXPECT_EQ(s.At(0), 1);
	EXPECT_EQ(s.At(1), 2);
	EXPECT_EQ(s.At(2), 3);
}

TEST(Shiftreg, TestBigShiftRef)
{
	ShiftReg<int16_t, 256> s;

	for (int i = 0; i < 128; i++) {
		s.Update(0, nullptr);
	}
}

TEST(Shiftreg, Convolute)
{
	int16_t initdata[3] = {0, 0, 1};
	ShiftReg<int16_t, 3> s(initdata);
	int16_t result;
	{
		int16_t initdata2[3] = {0, 0, 1};
		ShiftReg<int16_t, 3> t(initdata2);
		result = ShiftReg<int16_t, 3>::Convolute<int16_t,int16_t, 3, 3>(s, t);
		EXPECT_EQ(result, 1);
	}
	{
		int16_t initdata2[3] = {0, 1, 0};
		ShiftReg<int16_t, 3> t(initdata2);
		result = ShiftReg<int16_t, 3>::Convolute<int16_t,int16_t, 3, 3>(s, t);
		EXPECT_EQ(result, 0);
	}
	{
		int16_t initdata2[3] = {1, 0, 0};
		ShiftReg<int16_t, 3> t(initdata2);
		result = ShiftReg<int16_t, 3>::Convolute<int16_t,int16_t, 3, 3>(s, t);
		EXPECT_EQ(result, 0);
	}
	{
		int16_t initdata2[3] = {1, 1, -1};
		ShiftReg<int16_t, 3> t(initdata2);
		result = ShiftReg<int16_t, 3>::Convolute<int16_t,int16_t, 3, 3>(s, t);
		EXPECT_EQ(result, -1);
	}
}