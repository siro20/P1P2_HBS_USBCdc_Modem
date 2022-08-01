
#pragma once

class UARTEdge
{
	public:
		UARTEdge(void) :
		Phase(0), probability_max(0), counter(0)
		{
		}

		void Reset(void) {
			this->Phase = 0;
			this->probability_max = 0;
			this->counter = 0;
		}

		void Update(const int16_t p) {
			if (p > this->probability_max) {
				this->probability_max = p;
				this->Phase = this->counter;
			}
			this->counter++;
		}

		uint8_t Phase;

	private:
		int16_t probability_max;
		uint8_t counter;
};


