# UART data

The packets received on the P1/P2 bus are decoded and send to the
USB CDC and hardware UART on the RPi 40 pin connector.

1. The baud rate is 115200 8N1 no parity
2. The data is tranfered as ASCII and hex encoded.
3. The hex encoding might have whitespace between the hex characters
4. Every line contains one paket.
5. Every line is terminated with '\n'
6. A line has one or two prefix(es) sperated by ':'
   * The first prefix is a timestamp in decimal format in msec.
   * The second prefix is a status byte signaling the modem
     state and transmission error.
   * The prefix can be an empty string ""
7. Lines starting with `#` or `;` must be ignored

## Fields

Every message has the following format:

    Timestamp : Status : Data ...

## Status

A zero status byte signals no error.
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

`00000001:00:AB CD DE EF`


* Timestamp: `00000001`
* Status: `00` - OK
* Data: `ABCDDEEF`

**Example 2:**

`::CDefabcd`


* Timestamp: ``
* Status: ``
* Data: `CDEFABCD`

**Example 3:**

`1234:03:`

* Timestamp: `1234`
* Status: `03` - Received packet with parity error
* Data: ``


## Captured data

Captured data from bus:

    158:00:00001001810100000000150040000008000018004030006d
    9208:00:400010018021013000180015005a00000000000040000073
    9263:00:0000111098000000000000cf
    9313:00:40001120ae2f780c8023141f90249810980c8100000000db
    9371:00:000012c002132f1701040000000000413403d8
    9422:00:40001240400000006300000000007f010400000000000042
    9477:00:00001300005004
    9523:00:4000133000035200000f00000075b2f6c1000004
    9591:00:0000142d0012002d000700000000000000001f
    9641:00:4000142d0012002d000700000000000000001d00250052
    9681:00:00001500002300051459
    9719:00:40001500000a8000000f000010
    9772:00:000016000032140000000000000000000000005c
    9811:00:4000160000000000000000e106
    9872:00:00f0300100000002010000000000000000ec
    9919:00:40f03000000000010000010000000000007a
    9968:00:00f031000000018005170104132f3515
    10135:00:00f035240000250000260000270001280000290000ee
    10315:00:00f03577010f780100790100ffffffffffffffffff17
    10497:00:00f03614005e01ffffffffffffffffffffffffffffffffd8
    10675:00:00f038ffffffffffffffffffffffffffffffffffff8a
    10858:00:00001001810100000000150040000008000018004030006d
    10908:00:400010018021013000180015005a00000000000040000073
