# UART data

The packets received on the P1/P2 bus are decoded and send to the
USB CDC and hardware UART on the RPi 40 pin connector.

1. The baud rate is 115200 8N1 no parity
2. The data is tranfered as ASCII and hex encoded.
3. The hex encoding might have whitespace between the hex characters
4. Every line contains one paket.
5. Every line is terminated with '\n'
6. Everything after `#` or `;` must be ignored
7. On error a status is send, separated by `#`
   - The status indicates which kind of error happened
   - Since an error happened the data is likely corrupted.
8. Lines starting with `#` or `;` must be ignored

## Fields

Every message has the following format:

   Hex Data [ # Status ]

## Status

A zero status byte signals no error. Zero status is never transmitted.
On non zero status byte signals an error has happened.
One of the following errors are signaled:

* 1 : STATUS_ERR_BUS_COLLISION
  *  Did not RX the exact same data that was TXed
* 2 : STATUS_ERR_OVERFLOW
  * A buffer overflowed
* 3 : STATUS_ERR_PARITY
  * Parity error
* 4 : STATUS_ERR_NO_FRAMING
  * Did not detect line idle within timeout. Just receiving noise?

## Examples

**Example 1:**

`AB CD DE EF`

* Status: `00` - OK
* Data: `ABCDDEEF`

**Example 2:**

`CDefabcd # 01`

* Status: `1`
* Data: `CDEFABCD`


## Captured data

Captured data from bus:

    00001001810100000000150040000008000018004030006d
    400010018021013000180015005a00000000000040000073
    0000111098000000000000cf
    40001120ae2f780c8023141f90249810980c8100000000db
    000012c002132f1701040000000000413403d8
    40001240400000006300000000007f010400000000000042
    00001300005004
    4000133000035200000f00000075b2f6c1000004
    0000142d0012002d000700000000000000001f
    4000142d0012002d000700000000000000001d00250052
    00001500002300051459
    40001500000a8000000f000010
    000016000032140000000000000000000000005c
    4000160000000000000000e106
    00f0300100000002010000000000000000ec
    40f03000000000010000010000000000007a
    00f031000000018005170104132f3515
    00f035240000250000260000270001280000290000ee
    00f03577010f780100790100ffffffffffffffffff17
    00f03614005e01ffffffffffffffffffffffffffffffffd8
    00f038ffffffffffffffffffffffffffffffffffff8a
    00001001810100000000150040000008000018004030006d
    400010018021013000180015005a00000000000040000073
