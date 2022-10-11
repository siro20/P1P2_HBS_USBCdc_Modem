# PIO0 documentation

PIO0 is used as P1/P2 UART. It does CRC calculation, bit stuffing and AMI coding in hardware and thus offloads the
CPU. IRQ0 is used as busy indicator.

## Overview

The PIO program reads in 8bit data samples and transmits them using two GPIOs. The GPIOs are connected
to two equal sized resistors, encoding 0b00 as 0V, 0b11 as 3.3V and 0b10 as 1.65V.
This is enough for an AMI signal and can then be driven on the P1/P2 bus.

![](PIO0.png)

## The glue code

```C
    // Route pins to PIO
    pio_gpio_init(pio, pin_tx);
    pio_gpio_init(pio, pin_tx+1);

    // Disable pull on pins
    gpio_disable_pulls(pin_tx);
    gpio_disable_pulls(pin_tx+1);

    gpio_set_slew_rate(pin_tx, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(pin_tx+1, GPIO_SLEW_RATE_FAST);

    // Set pins to output
    pio_sm_set_consecutive_pindirs(pio, sm, pin_tx, 2, true/*output*/);

    pio_sm_config c = p1p2_uart_tx_program_get_default_config(offset);

    // OUT shifts to left, LSB first, no autopull
    sm_config_set_out_shift(&c, true, false, 8);

    // Set both pins as sideset pins
    sm_config_set_sideset_pins(&c, pin_tx);
    sm_config_set_sideset(&c, 2, false, false);
```
Configure GPIOs, set shift direction and set sideset pins.

```C
    // We only need TX, so get an 8-deep FIFO!
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
```
Disable RX FIFO and append it to the TX FIFO.

```
    // SM transmits 1 bit per 16 execution cycles.
    float div = (float)clock_get_hz(clk_sys) / (16 * baud);
    sm_config_set_clkdiv(&c, div);
```
Set the clock rate to 16x oversampling. As the maximum sideset delay is 7 (+1 for the instruction),
every bit must be encoded by two or more instruction.

```C
    // Clear IRQ flag before starting, and make sure flag doesn't actually
    // assert a system-level interrupt (we're using it as a status flag)
    pio_set_irq0_source_enabled(pio, isr, false);
    pio_set_irq1_source_enabled(pio, isr, false);
    pio_interrupt_clear(pio, sm);
```
Make sure to disable the interrupts. The IRQ bits are used to indicate that the
UART is busy.

```C
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

static inline void p1p2_uart_tx(PIO pio, uint sm, const uint8_t data) {
	pio_sm_put(pio, sm, data);
}
```
Start the PIO state machine. Also provide an better name for the TX method.
## The source

```C
.program p1p2_uart_tx

.side_set 2	; The two pins for an analog voltage but using two resistors

.define LEVEL_ONE       2 ; Analog voltage = 0.5 Vcc
.define LEVEL_IDLE      1 ; Same as ONE, but swapped (will be visible on oscilloscope)
.define LEVEL_ZERO_UP   3 ; Analog voltage = Vcc
.define LEVEL_ZERO_DOWN 0 ; Analog voltage = GND

```
Define the number of side-set bits. This reduces the possible side delay bits (maximum 7).
Defines the AMI levels driven on the sideset pins.


```C
public start:
next_byte:
    irq clear 0          side LEVEL_IDLE            ; clear IRQ while blocked
    pull block           side LEVEL_IDLE            ; send data bits until OSR empty

    ; send start bit
    irq set 0            side LEVEL_ZERO_UP    [7]  ; set IRQ while transmitting
    nop                  side LEVEL_IDLE       [5]  ; discard 24 bits to transmit 8 bits only

loop_odd:
    out y, 1             side LEVEL_IDLE            ; get next data bit 
    jmp !y, send_0_odd   side LEVEL_IDLE
send_1_odd:
    nop                  side LEVEL_ONE        [7]
    nop                  side LEVEL_IDLE       [3]
    jmp loop_end_odd     side LEVEL_IDLE            ; keep parity

send_0_odd:
    nop                  side LEVEL_ZERO_DOWN  [7]
    nop                  side LEVEL_IDLE       [3]
    jmp loop_end_even    side LEVEL_IDLE            ; toggle parity 

loop_even:
    out y, 1             side LEVEL_IDLE            ; get next data bit
    jmp !y, send_0_even  side LEVEL_IDLE
send_1_even:
    nop                  side LEVEL_ONE        [7]
    nop                  side LEVEL_IDLE       [3]
    jmp loop_end_even    side LEVEL_IDLE            ; keep parity
send_0_even:
    nop                  side LEVEL_ZERO_UP    [7]
    nop                  side LEVEL_IDLE       [3]
    jmp loop_end_odd     side LEVEL_IDLE            ; toggle parity 

loop_end_odd:
    jmp !osre, loop_odd  side LEVEL_IDLE
    nop                  side LEVEL_IDLE       [1]  ; finish idle phase of last bit
    nop                  side LEVEL_ZERO_DOWN  [7]  ; send '0' to make parity even
    nop                  side LEVEL_IDLE       [7]

    nop                  side LEVEL_ONE        [7]  ; send '1' as stop bit
    jmp next_byte        side LEVEL_IDLE       [4]  ; idle phase of stop bit, then jump

loop_end_even:
    jmp !osre, loop_even side LEVEL_IDLE
    nop                  side LEVEL_IDLE       [1]  ; finish idle phase of last bit
    nop                  side LEVEL_ONE        [7]  ; send '1' to keep parity even
    nop                  side LEVEL_IDLE       [7]

    nop                  side LEVEL_ONE        [7]  ; send '1' as stop bit
    jmp next_byte        side LEVEL_IDLE       [4]  ; idle phase of stop bit, then jump
```

The GPIOs are driven using the `side XXX` instruction as part of the regular programm flow.

Parity is calculated by jumping between `loop_end_even` and `loop_end_odd`. At the end of
data transmission this can be used to transmit the correct parity bit.
Usually parity is calculated on the '1' bit, but here it uses the '0' bit and thus can drive
the AMI code at the same time. On every '0' parity flips and AMI code flips as well.