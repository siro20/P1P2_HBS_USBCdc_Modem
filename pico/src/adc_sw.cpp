
#include <string.h>
#include <stdio.h>

#include "adc_sw.hpp"
#include "pico/platform.h"
#include "defines.hpp"

// Error returns true if there was an error since the last check
bool DifferentialADC_SW::Error(void) {
	bool val = this->error;
	this->error = false;
	return val;
}

// The gain is in 1/256th units.
// This allows to compensate external resistor dividor networks.
void DifferentialADC_SW::SetGain(uint16_t gain) {
	// 0xff is 3300mV -> adjust gain
	this->gain = ((uint32_t)gain * ADC_REF_VOLTAGE_MV) >> 8;
}

static void irq_adc_handler(void) {
	DifferentialADC_SW& dadc = DifferentialADC_SW::getInstance();
	dadc.ClearIRQ();
}

void DifferentialADC_SW::ClearIRQ(void) {
	this->rx_fifo.Push(adc_fifo_get());
}

// Configures ADC0 and ADC1 in round robin mode using DMA.
// It makes use of PIO1 to invert ADC0 samples in hardware.
// ADC1 samples are not inverted by the PIO.
// Calculates the phase correct differential signal by delaying
// the sampled data by one sample + a few CPU cycles used for the PIO.

DifferentialADC_SW::DifferentialADC_SW() :
	error(false), gain(0x100) {

	adc_init();

	// Make sure :GPIOs are high-impedance, no pullups etc
	adc_gpio_init(26);
	adc_gpio_init(27);

	// Select ADC input 0 (GPIO26)
	adc_select_input(0);

	adc_fifo_setup(
		true,    // Write each completed conversion to the sample FIFO
		true,    // Enable DMA data request (DREQ)
		1,       // DREQ (and IRQ) asserted when at least 1 sample present
		false,   // We won't see the ERR bit because of 8 bit reads; disable.
		true     // Shift each sample to 8 bits when pushing to FIFO
	);

	adc_set_clkdiv(48000000 / (UART_BAUD_RATE * ADC_OVERSAMPLING_RATE));
	adc_set_round_robin(0x3); // Sample ADC0 + ADC1 in RR

	// Configure the processor to run irq_adc_handler() when ADC0 IRQ is asserted
	irq_set_exclusive_handler(ADC_IRQ_FIFO, irq_adc_handler);
	irq_set_enabled(ADC_IRQ_FIFO, true);

	adc_irq_set_enabled(true);
}

void DifferentialADC_SW::Start(void) {
	adc_run(true);
}

void DifferentialADC_SW::Stop(void) {
	adc_run(false);
	adc_fifo_drain();
	adc_irq_set_enabled(false);
}

DifferentialADC_SW::~DifferentialADC_SW(void) {
	adc_run(false);
	adc_irq_set_enabled(false);
}

void DifferentialADC_SW::Reset(void) {
	this->Stop();
	this->Start();
}

// Update returns false if no new data is available.
// Update returns true if new data has been placed in out.
// out holds the sampled voltage in mV
bool DifferentialADC_SW::Update(int32_t *out) {
	int16_t x1, x2, y;
	int16_t diff;

	if (this->rx_fifo.Empty())
		return false;

	this->last_samples[2] = this->last_samples[1];
	this->last_samples[1] = this->last_samples[0];
	if (!this->rx_fifo.Pop(&this->last_samples[0]))
		return false;

	// Compensate phase shift. Use the last 3 samples. Intentionally overflows.
	x1 = this->last_samples[0];
	x2 = this->last_samples[2];
	y = this->last_samples[1];

	// Generate the average of x, which is the time-point of sampling y
	diff = (x1 + x2) / 2;
	// Add the inverting channel.
	diff -= y;

	// Apply gain to convert DAC value to mV
	*out = (diff * this->gain) >> 8;

	return true;
}

