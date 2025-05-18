#pragma once
#include "hardware/adc.h"
#include "hardware/pio.h"
#include "fifo_irqsafe.hpp"

using namespace std;

#define ADC_BUFFER_LEN 0x100

class DifferentialADC_SW
{
	public:
		static DifferentialADC_SW& getInstance(void)
		{
			static DifferentialADC_SW instance;
			return instance;
		}
		~DifferentialADC_SW(void);

		DifferentialADC_SW(DifferentialADC_SW const&) = delete;
		void operator=(DifferentialADC_SW const&) = delete;

		void Reset(void);

		bool Update(int32_t *out);
		bool Error(void);
		void SetGain(uint16_t gain);
		void Start(void);
		void Stop(void);

		void ClearIRQ(void);
	private:
		DifferentialADC_SW(void);

		/* ADC sample buffer */
		FifoIrqSafe<int16_t, 128> rx_fifo;

		bool error;
		int32_t gain;

		int16_t last_samples[3];
};
