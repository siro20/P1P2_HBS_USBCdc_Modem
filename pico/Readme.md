# RPI PICO P1/P2 modem source code

This project aims to access the 2-wire interface on Daikin heap pump systems called P1/P2.
It's inspired by [Arnold-n/P1P2Serial](https://github.com/Arnold-n/P1P2Serial), but builds
on a RPI hat using the RPI PICO as P1/P2 modem.

## Design

The modem uses ADC Chan1/Chan2 in round robin mode for RX and PIO0 for TX.

While the TX part needs no CPU cycles (except pushing to the PIO TX FIFO), the
RX path occupies the whole Cortex-M0+ CPU core.

## Build instructions

The code makes use of the [pico-sdk](https://github.com/raspberrypi/pico-sdk).
It's linked as submodule so you need to check it out first:

```
cd pico
git submodule update --init --checkout --recursive
cmake .
make
```

It generates an U2F file in `pico/src/p1p2.uf2` that can be copied to the
USB mass storage devices emulated by the Pico.

### Stats TX
- 16x Oversampling on TX

### Stats RX
- 32x Differential oversampling on RX providing 9bit
- 7-step FIR filter
- Resampler after FIR to 16x Oversampling
- P1/P2 decoder detecting parity, frame and polarity errors
