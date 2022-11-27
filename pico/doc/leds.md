# LED manager
The pico drives 3 LEDs to signal its current operating state.
There are
* TX LED
* RX LED
* Power LED

Every LED can be one of [OFF, ON, Slowly BLINKING, Fast BLINKING].

## Decoding LED states

**Board is idle:**
```
RX  off
PWR on
TX  off
```

**Board is RXing data:**
```
RX  slowly blinking
PWR on
TX  off
```

**Board is TXing data: (impossible state)**

(The board should always RX data when it's TXing.)
```
RX  off
PWR on
TX  slowly blinking
```

**Board is RX/TXing data:**
```
RX  slowly blinking
PWR on
TX  slowly blinking
```

**Board has RX data with parity error:**
```
RX  fast blinking
PWR fast blinking
TX  off
```

**Board transmitted data, but a bus collision happened:**
```
RX  off
PWR fast blinking
TX  fast blinking
```

**Internal error:**

ADC underrun, FIFO overrun, Line busy for too long, UART TX buffer overflow, ...
```
RX  off
PWR fast blinking
TX  off
```

