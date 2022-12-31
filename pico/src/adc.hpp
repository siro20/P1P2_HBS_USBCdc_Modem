#pragma once
#include "hardware/adc.h"
#include "hardware/pio.h"

using namespace std;

#define ADC_BUFFER_LEN 0x100

class DifferentialADC
{
	public:
		static DifferentialADC& getInstance(int16_t data_ptr[ADC_BUFFER_LEN])
		{
			static DifferentialADC instance(data_ptr);
			return instance;
		}
		~DifferentialADC(void);

		DifferentialADC(DifferentialADC const&) = delete;
		void operator=(DifferentialADC const&) = delete;

		void Reset(void);

		bool Update(int32_t *out);
		bool Error(void);
		void SetGain(uint16_t gain);
		void Start(void);
		void Stop(void);

		void AckDMAIRQ(void);
		void DMARestart(void);
	private:
		DifferentialADC(int16_t data_ptr[ADC_BUFFER_LEN]);

		/* ADC sample buffer */
		int16_t *data;

		uint32_t tc;
	
		uint8_t off;
		bool error;
		int32_t gain;
		// PIO
		PIO pio;
		uint sm;

		// DMA channels
		int channel;
		int channel2;
};
