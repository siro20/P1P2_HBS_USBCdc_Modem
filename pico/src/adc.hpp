#pragma once
#include "hardware/adc.h"
#include "hardware/pio.h"

using namespace std;

class DifferentialADC
{
	public:
		static DifferentialADC& getInstance(void)
		{
			static DifferentialADC instance;
			return instance;
		}
		~DifferentialADC(void);

		DifferentialADC(DifferentialADC const&) = delete;
		void operator=(DifferentialADC const&) = delete;

		void Reset(void);

		bool Update(int16_t *out);
		bool Error(void);
		void SetGain(uint16_t gain);
		void Start(void);
		void Stop(void);

		void AckDMAIRQ(void);
	private:
		DifferentialADC(void);

		/* ADC sample buffer */
		int16_t data[0x100] __attribute__ ((aligned(0x200)));

		uint32_t tc;
	
		uint8_t off;
		bool error;
		int16_t gain;
		// PIO
		PIO pio;
		uint sm;

		// DMA channels
		int channel;
		int channel2;
};