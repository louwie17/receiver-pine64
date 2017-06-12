## Receiver for Pine A64

This is simple C application receives using a radio receiver module.

### Compile

Compile directly on Pine A64.

### Use it

Find a PINE GPIO number: http://joey.hazlett.us/pine64/pine64_pins.html
You can use either BCM pin numbers or the Pine64 gpio numbers

Execute in terminal:

```bash
receiver -p <pin-number>
```

It will read from the receiver continually using the RCSwitch.

### Dependencies

For controlling the GPIO's I used this RPi.GPIO-PineA64:
https://github.com/swkim01/RPi.GPIO-PineA64/blob/master/source/c_gpio.c

The piHiPri code comes from https://github.com/WiringPi/WiringPi
I also modified the wiringPiISR from wiringPi, to make it work for the PINE64

## Author

Lourens Schep
