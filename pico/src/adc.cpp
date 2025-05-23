
#include <string.h>
#include <stdio.h>

#include "adc.hpp"
#include "hardware/dma.h"
#include "pico/platform.h"
#include "twos_complement.pio.h"
#include "defines.hpp"

// Error returns true if there was an error since the last check
bool DifferentialADC::Error(void) {
	bool val = this->error;
	this->error = false;
	return val;
}

// The gain is in 1/256th units.
// This allows to compensate external resistor dividor networks.
void DifferentialADC::SetGain(uint16_t gain) {
	// 0xff is 3300mV -> adjust gain
	this->gain = ((uint32_t)gain * ADC_REF_VOLTAGE_MV) >> 8;
}

static void irq_dma_handler(void) {
	DifferentialADC& dadc = DifferentialADC::getInstance(NULL);

	dadc.AckDMAIRQ();
}

void DifferentialADC::AckDMAIRQ(void) {
	// clear interrupt request on the executing core
	if (dma_channel_get_irq0_status(this->channel2))
		dma_channel_acknowledge_irq0(this->channel2);
	if (dma_channel_get_irq0_status(this->channel))
		dma_channel_acknowledge_irq0(this->channel);

	// Nothing to do here. it just wakes the CPU from _wfe.
}

// Configures ADC0 and ADC1 in round robin mode using DMA.
// It makes use of PIO1 to invert ADC0 samples in hardware.
// ADC1 samples are not inverted by the PIO.
// Calculates the phase correct differential signal by delaying
// the sampled data by one sample + a few CPU cycles used for the PIO.

DifferentialADC::DifferentialADC(int16_t data_ptr[ADC_BUFFER_LEN]) : data(data_ptr),
	tc(0x10000000), off(0), error(false), gain(0x100), pio(pio1), sm(0) {

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

	// Load the PIO.
	// It converts the uint8_t to int16_t and
	// it inverts the even channel and passthrough the odd channel.
	// The Update() routing thus only need to add the channels without careing
	// about the channel polarity.
	// The PIO takes on average 40 cpu cycles to calculate the inverse.
	uint offset = pio_add_program(this->pio, &twos_complement_program);
	twos_complement_program_init(this->pio, this->sm, offset);

	
	// Get a free channel, panic() if there are none
	this->channel2 = dma_claim_unused_channel(true);
	// Get a free channel, panic() if there are none
	this->channel = dma_claim_unused_channel(true);

	{
		dma_channel_config c = dma_channel_get_default_config(this->channel2);
		// 16 bit transfers
		channel_config_set_transfer_data_size(&c, DMA_SIZE_16);
		// increment the write adddress, don't increment read address
		channel_config_set_read_increment(&c, false);
		channel_config_set_write_increment(&c, true);

		channel_config_set_high_priority(&c, true);

		// Pace transfers based on availability of PIO samples
		channel_config_set_dreq(&c, pio_get_dreq(this->pio, this->sm, false));

		// set wrap boundary. This is why we needed alignment!
		channel_config_set_ring(&c, true, 9); // 1 << 9 byte boundary on write ptr

		dma_channel_configure(this->channel2, &c,
			this->data,     // dst
			twos_complement_dma_rx_reg(this->pio, this->sm),  // src
			this->tc,       // transfer count
			false           // start immediately
		);
	}
	{
		dma_channel_config c = dma_channel_get_default_config(this->channel);
		// 8 bit transfers
		channel_config_set_transfer_data_size(&c, DMA_SIZE_8);
		//  don't increment read and write address
		channel_config_set_read_increment(&c, false);
		channel_config_set_write_increment(&c, false);
	
		channel_config_set_high_priority(&c, true);

		// Pace transfers based on availability of ADC samples
		channel_config_set_dreq(&c, DREQ_ADC);

		dma_channel_configure(this->channel, &c,
			twos_complement_dma_tx_reg(this->pio, this->sm),  // dst
			&adc_hw->fifo,  // src
			this->tc,       // transfer count
			false           // start immediately
		);
	}
}

void DifferentialADC::DMARestart(void) {
	this->off = 0;
	this->tc = 0x10000000;

	// Set initial write address. Register is incremented on each transfer.
	// If Stop is called before transfer count is reached, off would be out
	// of sync to hardware pointer.

	dma_channel_set_write_addr(this->channel2, this->data, false);

	dma_channel_set_trans_count(this->channel, this->tc, false);
	dma_channel_set_trans_count(this->channel2, this->tc, false);

	dma_start_channel_mask((1 << this->channel)|(1 << this->channel2));
}

void DifferentialADC::Start(void) {

	// Configure the processor to run irq_dma_handler() when DMA IRQ 0 is asserted
	// PICOSDK bug! Must not be part of constructor! constructor is run on core0.
	irq_set_exclusive_handler(DMA_IRQ_0, irq_dma_handler);
	irq_set_enabled(DMA_IRQ_0, true);

	// Prepare for IRQ support
	twos_complement_prepare_irq0(this->pio, this->sm);

	pio_sm_restart(this->pio, this->sm);
	pio_sm_set_enabled(this->pio, this->sm, true);

	this->DMARestart();

	// Tell the DMA to raise IRQ line 0 when the channel finishes a block
	dma_channel_set_irq0_enabled(this->channel2, true);
	dma_channel_set_irq0_enabled(this->channel, true);

	// This one wakes the CPU on WFE
	twos_complement_enable_irq0(this->pio, this->sm);
	adc_run(true);
}

void DifferentialADC::Stop(void) {
	adc_run(false);
	adc_fifo_drain();

	twos_complement_disable_irq0(this->pio, this->sm);
	dma_channel_set_irq0_enabled(this->channel2, false);
	dma_channel_set_irq0_enabled(this->channel, false);
	dma_channel_abort(this->channel);
	dma_channel_abort(this->channel2);
	pio_sm_set_enabled(this->pio, this->sm, false);
	pio_sm_clear_fifos(this->pio, this->sm);

	twos_complement_unprepare_irq0(this->pio, this->sm);

	irq_set_enabled(DMA_IRQ_0, false);
	irq_set_exclusive_handler(DMA_IRQ_0, NULL);
}

DifferentialADC::~DifferentialADC(void) {
	adc_run(false);
	dma_channel_abort(this->channel);
	dma_channel_unclaim(this->channel);
	dma_channel_abort(this->channel2);
	dma_channel_unclaim(this->channel2);
}

void DifferentialADC::Reset(void) {
	this->Stop();
	this->Start();
}

// Update returns false if no new data is available.
// Update returns true if new data has been placed in out.
// out holds the sampled voltage in mV
bool DifferentialADC::Update(int32_t *out) {
	uint32_t tc_hw;
	int16_t x1, x2, y;
	int16_t diff;
	int32_t timeout = 100000;

	tc_hw = dma_channel_hw_addr(this->channel2)->transfer_count;
	if (tc_hw == 0) {
		//
		// Need to check for transfer end here to avoid
		// race conditions between interrupt code and Update().
		// The IRQ handler will only wake the CPU from _wfe.
		//
		while (dma_channel_hw_addr(this->channel)->transfer_count != 0) {
			// channel should finish before channel2.
			// This code should never be entered.
			timeout--;
			if (timeout == 0) {
				this->error = true;
				break;
			}
		}

		this->DMARestart();
		tc_hw = dma_channel_hw_addr(this->channel2)->transfer_count;
	}

	// Compare transfer count with internal shadow register
	if (this->tc == tc_hw)
		return false;
	// Got data!

	// Check for overrun
	if ((this->tc - tc_hw) & 0xffffff00)
		this->error = true;

	this->tc--;

	// Compensate phase shift. Use the last 3 samples. Intentionally overflows.
	x1 = this->data[(uint8_t)(this->off - 0)];
	x2 = this->data[(uint8_t)(this->off - 2)];
	y = this->data[(uint8_t)(this->off - 1)];

	// Generate the average of x, which is the time-point of sampling y
	diff = (x1 + x2) / 2;
	// Add the inverting channel. The PIO has already negated the value in hardware.
	diff += y;

	// Intentionally overflows.
	this->off++;

	// Apply gain to convert DAC value to mV
	*out = (diff * this->gain) >> 8;

	return true;
}

