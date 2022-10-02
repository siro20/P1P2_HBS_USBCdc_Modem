
#pragma once
#include "defines.hpp"

template <class T>
class Level
{
  public:

    Level() : state{0} {}

    // Update returns false if no new data is available.
    // Update returns true if new data has been placed in out.
    bool Update(const T in, T *out)  {
        // Detect level and apply hysteresis
        if (this->state == 0) {
            if (in >= BUS_HIGH_MV) {
                this->state = 1;
            } else if (in <= -BUS_HIGH_MV) {
                this->state = -1;
            }
        } else if (this->state == 1) {
            if (in < BUS_LOW_MV) {
                this->state = 0;
            }
        } else {
            if (in > -BUS_LOW_MV) {
                this->state = 0;
            }
        }
        if (this->state == 0) {
                // Reduce signal amplitude if '0' has been detected
                // Divide by 10.
                *out = (in * 26) >> 8;
        } else {
                *out = in;
        }
        return true;
    }

  private:
    T state;
};
