# KNX incubator project

## KNX bus sniffer using Arduino Core on Maple Mini STM32

Schematic: https://easyeda.com/pavel.kriz.cz/knx-bus-sniffer

### Osciloscope measurements

![scope1](doc/scope1.png?raw=true)

![scope2](doc/scope2.png?raw=true)

![scope3](doc/scope3.png?raw=true)

![scope4](doc/scope4.png?raw=true)

### Sample logs from the sniffer

#### Acked Telegram
```
========== KNX TELEGRAM ==========
RAW: 188 17 10 0 1 225 0 128 56 
CTR: prio_normal_low not_repeated standard_frame
SRC: 1.1.10
DST: 0/0/1
NPDU: routing=6 length=1
CMD: value write 0
========== GAP ==========
1660 us
========== KNX TELEGRAM ==========
RAW: 204 
ACK: positive
```

#### Not-Acked (Repeated) Telegram
```
========== KNX TELEGRAM ==========
RAW: 188 17 10 0 1 225 0 129 57 
CTR: prio_normal_low not_repeated standard_frame
SRC: 1.1.10
DST: 0/0/1
NPDU: routing=6 length=1
CMD: value write 1
========== GAP ==========
16259 us
========== KNX TELEGRAM ==========
RAW: 156 17 10 0 1 225 0 129 25 
CTR: prio_normal_low repeated standard_frame
SRC: 1.1.10
DST: 0/0/1
NPDU: routing=6 length=1
CMD: value write 1
========== GAP ==========
16266 us
========== KNX TELEGRAM ==========
RAW: 156 17 10 0 1 225 0 129 25 
CTR: prio_normal_low repeated standard_frame
SRC: 1.1.10
DST: 0/0/1
NPDU: routing=6 length=1
CMD: value write 1
========== GAP ==========
16253 us
========== KNX TELEGRAM ==========
RAW: 156 17 10 0 1 225 0 129 25 
CTR: prio_normal_low repeated standard_frame
SRC: 1.1.10
DST: 0/0/1
NPDU: routing=6 length=1
CMD: value write 1
```

## Acknowledgement

This work is supported by [hkfree.org](http://www.hkfree.org) community network.