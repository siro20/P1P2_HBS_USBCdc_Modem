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

## Status
A zero status byte signals no error.
On non zero status byte signals an error has happened.
One of the following errors are signaled:

* 1 : STATUS_ERR_BUS_COLLISION
   Did not RX the exact same data that was TXed
* 2 : STATUS_ERR_OVERFLOW
  A buffer overflowed
* 3 : STATUS_ERR_PARITY
  Parity error
* 4 : STATUS_ERR_NO_FRAMING
  Did not detect line idle within timeout. Just receiving noise?

## Examples

**Example 1:**
```
00000001:00:AB CD DE EF
```

* Timestamp: `00000001`
* Status: `00` - OK
* Data: `ABCDDEEF`

**Example 2:**
```
::CDefabcd
```

* Timestamp: ``
* Status: ``
* Data: `CDEFABCD`

**Example 3:**
```
1234:03:
```

* Timestamp: `1234`
* Status: `03` - Received packet with parity error
* Data: ``

