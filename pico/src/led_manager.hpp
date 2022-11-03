
#pragma once
#include "pico/time.h"
#include "led_driver.hpp"

class LEDManager
{
	public:
		enum LEDState {
			LED_IDLE = 0,
			LED_RX_ACTIVITY,
			LED_TX_ACTIVITY,
			LED_RX_TX_ACTIVITY,
			LED_BUS_ERROR,
			LED_SYSTEM_ERROR,
			LED_TRANSMISSION_RX_ERROR,
			LED_TRANSMISSION_TX_ERROR,
		};

		LEDManager(LEDdriver &rx, LEDdriver &tx, LEDdriver &power) :
		state(LED_SYSTEM_ERROR), newState(LED_IDLE), r(&rx), t(&tx), p(&power)
		{
			add_repeating_timer_ms (500, LEDManagerTimerCallback, this, &this->timer);
			this->p->Set(LEDdriver::LED_ON);
			this->r->Set(LEDdriver::LED_ON);
			this->t->Set(LEDdriver::LED_ON);
		}

		~LEDManager(void)
		{
			cancel_repeating_timer(&this->timer);
		}

		void ActivityRx(void) {
			if (this->newState == LED_IDLE) {
				this->newState = LED_RX_ACTIVITY;
			} else if (this->newState == LED_TX_ACTIVITY) {
				this->newState = LED_RX_TX_ACTIVITY;
			}
		}

		void ActivityTx(void) {
			if (this->newState == LED_IDLE) {
				this->newState = LED_TX_ACTIVITY;
			} else if (this->newState == LED_RX_ACTIVITY) {
				this->newState = LED_RX_TX_ACTIVITY;
			}
		}

		// Data receival failed
		void TransmissionErrorRx(void) {
			if (this->newState != LED_SYSTEM_ERROR && this->newState != LED_BUS_ERROR) {
				this->newState = LED_TRANSMISSION_RX_ERROR;
			}
		}

		// Data transmission failed
		void TransmissionErrorTx(void) {
			if (this->newState != LED_SYSTEM_ERROR && this->newState != LED_BUS_ERROR) {
				this->newState = LED_TRANSMISSION_TX_ERROR;
			}
		}

		// System error occured
		void InternalError(void) {
			this->newState = LED_SYSTEM_ERROR;
		}

		// Bus error occured (no DC power)
		void BusError(void) {
			this->newState = LED_BUS_ERROR;
		}

	private:
		static bool LEDManagerTimerCallback(repeating_timer_t *rt)
		{
			LEDManager *manager = static_cast<LEDManager*>(rt->user_data);

			manager->Update();

			return true;
		}

		inline void Update(void) {
			this->state = this->newState;
			this->newState = LED_IDLE;

			switch (this->state) {
			case LED_IDLE:
				this->p->Set(LEDdriver::LED_ON);
				this->r->Set(LEDdriver::LED_OFF);
				this->t->Set(LEDdriver::LED_OFF);
				break;
			case LED_BUS_ERROR:
				this->p->Set(LEDdriver::LED_OFF);
				this->r->Set(LEDdriver::LED_ON);
				this->t->Set(LEDdriver::LED_ON);
				break;
			case LED_RX_ACTIVITY:
				this->p->Set(LEDdriver::LED_ON);
				this->r->Set(LEDdriver::LED_BLINK_SLOW);
				this->t->Set(LEDdriver::LED_OFF);
				break;
			case LED_TX_ACTIVITY:
				this->p->Set(LEDdriver::LED_ON);
				this->r->Set(LEDdriver::LED_OFF);
				this->t->Set(LEDdriver::LED_BLINK_SLOW);
				break;
			case LED_RX_TX_ACTIVITY:
				this->p->Set(LEDdriver::LED_ON);
				this->r->Set(LEDdriver::LED_BLINK_SLOW);
				this->t->Set(LEDdriver::LED_BLINK_SLOW);
				break;
			case LED_SYSTEM_ERROR:
				this->p->Set(LEDdriver::LED_BLINK_FAST);
				this->r->Set(LEDdriver::LED_OFF);
				this->t->Set(LEDdriver::LED_OFF);
				break;
			case LED_TRANSMISSION_TX_ERROR:
				this->p->Set(LEDdriver::LED_BLINK_FAST);
				this->r->Set(LEDdriver::LED_OFF);
				this->t->Set(LEDdriver::LED_BLINK_FAST);
				break;
			case LED_TRANSMISSION_RX_ERROR:
				this->p->Set(LEDdriver::LED_BLINK_FAST);
				this->r->Set(LEDdriver::LED_BLINK_FAST);
				this->t->Set(LEDdriver::LED_OFF);
				break;
			}
		}

		enum LEDState state;
		enum LEDState newState;
		LEDdriver *r;
		LEDdriver *t;
		LEDdriver *p;
		repeating_timer_t timer;
};


