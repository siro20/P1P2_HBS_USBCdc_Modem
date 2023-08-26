# Audio IC shutdown and wakeup

The audio amplifier IC, as descibed in the [circuits section](circuits.md),
is shut down on RX and only enabled when something needs to be transmitted.

This also ensure high impedance seen on the remote bus transmitter. It's
critical that the shutdown and power on doesn't cause huge differential
voltage spikes. Spikes could be interpreted by the remote as start of
new packet.


## Ti LM4871

The Ti LM4871 has a startup time of 15msec. During the startup time there's
a huge distortion at the output, commonly refered to as "clicks" or "pops".

The following sample shows the traffic on the P1/P2 bus, as well as the
power off and power on behaviour.
While power off is OK, as the differential
voltage is 0, the turn on click is unacceptable.

![](DS1Z_LM4871.png)

**Experiments showed that the output drives at full power with less than 4 Ohms
impedance. An external circuit isn't capable of keeping the differential
voltage low.**

## STM TS4990

The STM TS4990 is a pin compatible audio amplifier IC that advertises the
"Near-zero pop and click" feature. The STM TS4990 has a startup time of 32
msec.

The following sample shows the traffic on the P1/P2 bus, as well as the
power off and power on behaviour

![](DS1Z_TS4990.png)
