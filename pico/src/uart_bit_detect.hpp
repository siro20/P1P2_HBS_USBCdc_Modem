#pragma once
#include "shiftreg.hpp"
#include "math.h"

// reverse?

#define OVERSAMPLING 16

static const int8_t uart_detect_abs[OVERSAMPLING] = {
    -1, 0, 0, 0, 0, 0, 0, 0, 0, -1, -1, -1, -1, -1, -1, -1
};

static const int8_t uart_detect_high[OVERSAMPLING] = {
    0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0
};

static const int8_t uart_detect_low[OVERSAMPLING] = {
    0, -1, -1, -1, -1, -1, -1, -1, -1, 0, 0, 0, 0, 0, 0, 0
};

class UARTBit
{
	public:
		UARTBit(void) :
		reg(), reg_absolute(),
		absolute(uart_detect_abs), high(uart_detect_high),
		low(uart_detect_low)
		{
		}

		// Update returns false if no new data is available.
		// Update returns true if new data has been placed in out.
		// The signal delay is equal to the convolution window size -1
		bool Update(const int16_t in, int16_t *probability_high, int16_t *probability_low) {

			int16_t val;
			int result_abs, result_high, result_low;

			// Shift in new value
			this->reg.Update(in, NULL);
			val = abs(in);
			this->reg_absolute.Update(val, NULL);

			// Compute probability for bit
			result_abs = ShiftReg<int16_t, OVERSAMPLING>::Convolute<int16_t,int8_t, OVERSAMPLING, OVERSAMPLING>(this->reg_absolute, this->absolute);
			result_low = ShiftReg<int16_t, OVERSAMPLING>::Convolute<int16_t,int8_t, OVERSAMPLING, OVERSAMPLING>(this->reg, this->low);
			result_high = ShiftReg<int16_t, OVERSAMPLING>::Convolute<int16_t,int8_t, OVERSAMPLING, OVERSAMPLING>(this->reg, this->high);

			*probability_high = result_abs + result_high;
			*probability_low = result_abs + result_low;

			return true;
		}

	private:
		ShiftReg<int16_t, OVERSAMPLING> reg;
		ShiftReg<int16_t, OVERSAMPLING> reg_absolute;

		ShiftReg<int8_t, OVERSAMPLING> absolute;
		ShiftReg<int8_t, OVERSAMPLING> high;
		ShiftReg<int8_t, OVERSAMPLING> low;
};


