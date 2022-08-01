
#pragma once
#include "pico/time.h"

class LEDdriver
{
	public:
		enum LEDState {
			LED_OFF = 0,
			LED_ON,
			LED_BLINK_SLOW,
			LED_BLINK_FAST,
		};

		static bool LEDTimerCallback(repeating_timer_t *rt)
		{
			LEDdriver *driver = static_cast<LEDdriver*>(rt->user_data);

			driver->Update();

			return true;
		}

		LEDdriver(int p) :
		state(LED_OFF), pin(p), counter(0)
		{
			add_repeating_timer_ms (100, LEDTimerCallback, this, &this->timer);

			gpio_init(p);
			gpio_set_dir(p, GPIO_OUT);
			gpio_put(p, false);
		}

		~LEDdriver(void)
		{
			cancel_repeating_timer(&this->timer);
		}

		void Set(enum LEDState s) {
			this->state = s;
		}

	private:

		void Update(void) {
			switch (this->state) {
				case LED_OFF:
					this->Off();
					break;
				case LED_ON:
					this->On();
					break;
				case LED_BLINK_SLOW:
					this->counter++;
					if (this->counter == 5) {
						this->counter = 0;
						this->Toggle();
					}
					break;
				case LED_BLINK_FAST:
					this->Toggle();
					break;
			}
		}

		void On(void) {
			gpio_put(this->pin, true);
		}

		void Off(void) {
			gpio_put(this->pin, false);
		}

		void Toggle(void) {
			gpio_put(this->pin, !gpio_get(this->pin));
		}

		enum LEDState state;
		repeating_timer_t timer;
		int pin;
		uint8_t counter;
};


