#pragma once
#include "hardware/adc.h"
#include "hardware/pio.h"
#include "fifo_irqsafe.hpp"

using namespace std;

#define ADC_BUFFER_LEN 0x100

class DifferentialADC_SW
{
	public:
		static DifferentialADC_SW& getInstance(uint16_t data_ptr[ADC_BUFFER_LEN])
		{
			static DifferentialADC_SW instance(data_ptr);
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
		DifferentialADC_SW(uint16_t data_ptr[ADC_BUFFER_LEN]);

		/* ADC sample buffer */
		volatile uint16_t *data;
		uint8_t off_tx;
		uint8_t off_rx;
	
		bool error;
		int32_t gain;

		bool chan_polarity;
		uint32_t last_samples[2];
};
