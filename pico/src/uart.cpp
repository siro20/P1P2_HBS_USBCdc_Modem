#include <inttypes.h>
#include "uart.hpp"

#include <iostream>

// Level sets the line level for a "high" or "low" symbol
// p is the parity.
// For P1P2 bus this must be 'PARITY_EVEN' to make sure the signal has a zero DC level.
UART::UART(enum UART_PARITY p) :
	parity(p),
	counter(0),
	state(WAIT_FOR_IDLE),
	reg()
{
}

// Receiving returns true as long as data is being received
bool UART::Receiving(void) {
	return this->state != WAIT_FOR_START;
}

inline bool UART::ZeroLevelDetect(const int16_t prob) {
	return prob != 0;
}

bool UART::FindBestPhase(uint8_t *out, bool *err) {
	uint32_t bestprob = 0;
	//std::cout << "FindBestPhase " << std::endl;

	// The START bit detected a zero condition, so the phase must be close to the
	// beginning. It's safe to ignore the 1/2 last part of the symbol.
	for (uint8_t phase = 0; phase < UART_OVERSAMPLING_RATE / 2; phase++) {
		uint8_t tmp_data, tmp_parity;
		bool rx_error;
		uint32_t prob;
		//std::cout << "phase: " << (int)phase << std::endl;
		tmp_data = 0;
		tmp_parity = 0;
		rx_error = false;

		if (this->parity == PARITY_NONE)
			prob = this->ExtractData(phase, &tmp_data, &rx_error);
		else {
			prob = this->ExtractDataAndParity(phase, &tmp_parity, &tmp_data, &rx_error);

			if ((tmp_parity & 1) && this->parity == PARITY_EVEN){
				//std::cout << "parity error " << std::endl;
				rx_error = true;
			}
			else if (!(tmp_parity & 1) && this->parity == PARITY_ODD) {
				//std::cout << "parity error " << std::endl;
				rx_error = true;
			}
		}

		if (prob > bestprob) {
			bestprob = prob;
			*out = tmp_data;
			*err = rx_error;
		}
		//std::cout << "prob " << (int) prob << std::endl;
		//std::cout << "rx_error " << (int) rx_error << std::endl;
		//std::cout << "data " << (int) tmp_data << std::endl;
		//std::cout << "parity " << (int) tmp_parity << std::endl;
		//std::cout << std::endl;
	}
	if (bestprob == 0) {
		*err = true;
	}
	return false;
}

uint32_t UART::ExtractData(const uint8_t phase, uint8_t *out, bool *err) {
	uint32_t prob;
	int16_t last_prob = this->reg.At(phase + 1 * UART_OVERSAMPLING_RATE);

	if (last_prob == 0) {
		// Framing error
		*err = true;
		return 0;
	}
	prob = abs(last_prob);

	for (uint8_t b = 1; b < UART_BITS_NO_PARITY; b++) {
		int16_t tmp_prob = this->reg.At(phase + (b + 1) * UART_OVERSAMPLING_RATE);
		// Decode data
		if (b >= 1 && b <= 8) {
			if (tmp_prob == 0) {
				*out |= 1 << (b - 1);
			}
		} else if (b == 9) {
			if (tmp_prob != 0) {
				// Framing error.
				*err = true;
			}
		}

		// Verify polarity. Errors might indicate a bus collision.
		if (tmp_prob != 0) {
			if ((last_prob > 0 && tmp_prob > 0) || (last_prob < 0 && tmp_prob < 0)) {
				*err = true;
			}

			last_prob = tmp_prob;
		}

		prob += abs(tmp_prob);
	}

	return prob;
}

uint32_t UART::ExtractDataAndParity(const uint8_t phase, uint8_t *parity, uint8_t *out, bool *err) {
	uint32_t prob;
	int16_t last_prob;
	
	*parity = 0;

	last_prob = this->reg.At(phase + 0 * UART_OVERSAMPLING_RATE);
	if (last_prob == 0) {
		// Framing error
		*err = true;
		return 0;
	}
	prob = abs(last_prob);

	for (size_t b = 1; b < UART_BITS_PARITY; b++) {
		int16_t tmp_prob = this->reg.At(phase + b * UART_OVERSAMPLING_RATE);
		// Decode data
		if (b >= 1 && b <= 8) {
			if (tmp_prob == 0) {
				*out |= 1 << (b - 1);
				(*parity)++;
			}
		} else if (b == 9) {
			if (tmp_prob == 0) {
				(*parity)++;
			}
		} else if (b == 10) {
			if (tmp_prob != 0) {
				// Framing error.
				*err = true;
			}
		}

		// Verify polarity. Errors might indicate a bus collision.
		if (tmp_prob != 0) {
			if ((last_prob > 0 && tmp_prob > 0) || (last_prob < 0 && tmp_prob < 0)) {
				*err = true;
			}
			last_prob = tmp_prob;
		}
		prob += abs(tmp_prob);

	}

	return prob;
}

// Update returns false if no new data is available.
// Update returns true if new data has been placed in out.
bool UART::Update(const int32_t symbol_prob, uint8_t *out, bool *err) {
	bool ret = false;

	switch(this->state) {
	case WAIT_FOR_IDLE:
		if (!this->ZeroLevelDetect(symbol_prob)) {
			this->state = WAIT_FOR_START;
		}
		break;
	case WAIT_FOR_START:
		// As ZeroLevelDetect returns true the phase must be close to beginning
		// so it's safe to drop 1/2 Symbol (STOP symbol) here.
		if (this->ZeroLevelDetect(symbol_prob)) {
			this->state = DATA;
			if (this->parity == PARITY_NONE)
				this->counter = UART_OVERSAMPLING_RATE * UART_BITS_NO_PARITY;
			else
				this->counter = UART_OVERSAMPLING_RATE * UART_BITS_PARITY;
			this->counter -= UART_OVERSAMPLING_RATE/2;
		}
		// fallthrough
	case DATA:
		this->reg.Update(symbol_prob, nullptr);

		this->counter--;
		if (this->counter == 0) {
			this->state = STOP;
		}

		break;
	case STOP:
		//for (int i = 0; i < this->reg_parity.Length(); i++)
		//	std::cout << " i " << i << " data " << this->reg_parity.At(i) << std::endl;
		this->FindBestPhase(out, err);
		ret = true;
		this->state = WAIT_FOR_IDLE;
	
	break;
	}

	return ret;
}
