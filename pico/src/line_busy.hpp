#pragma once
#include "shiftreg.hpp"

template <size_t N>
class LineBusy
{
	public:
		LineBusy(const int32_t level) :
		buffer{0},
		reg(buffer),
		level(level)
		{
		}

		void Update(const int32_t in) {
			// Shift in new value
			this->reg.Update(in, NULL);
		}

		// Busy returns true if a "0" symbol is likely being transmitted on the line
		// This function is not as accurate as the uart_bit_detect
		bool Busy(void) {
			size_t cnt = 0;
			for (size_t i = 0; i < N; i++)
				if (this->reg.At(i) > level || this->reg.At(i) < -level)
					cnt ++;
			if (cnt > (N / 8) && cnt <= (N * 5 / 8))
				return true;
			return false;
		}

	private:
		int32_t buffer[N * 2];
		ShiftReg<int32_t, N> reg;
		int32_t level;
};


