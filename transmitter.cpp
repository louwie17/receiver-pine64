/*
 * http://joey.hazlett.us/pine64/pine64_pins.html
 */
#include "wiringPi.h"
#include "RCSwitch.h"
#include <stdio.h>
#include <stdlib.h>

int TRANSMITTER_PIN = 71;

int main(int argc, char *argv[])
{

    while (true) {
        switch (getopt(argc, argv, "d:r:p:s:")) {
            case -1:
                goto done;

            case 'p':
                TRANSMITTER_PIN = atoi(optarg);
                break;

            default: /* '?' */
                break;
        }
    }

    done:

    if (wiringPiSetup () == -1) return 1;
    printf("Using pin: %d\n", TRANSMITTER_PIN);
    RCSwitch mySwitch = RCSwitch();
    mySwitch.enableTransmit(TRANSMITTER_PIN);

    /* See Example: TypeA_WithDIPSwitches */
    mySwitch.switchOn("11111", "00010");
    delay(1000);
    mySwitch.switchOff("11111", "00010");
    delay(1000);

    /* Same switch as above, but using decimal code */
    mySwitch.send(5393, 24);
    delay(1000);
    mySwitch.send(5396, 24);
    delay(1000);

    /* Same switch as above, but using binary code */
    mySwitch.send("000000000001010100010001");
    delay(1000);
    mySwitch.send("000000000001010100010100");
    delay(1000);

    /* Same switch as above, but tri-state code */
    mySwitch.sendTriState("00000FFF0F0F");
    delay(1000);
    mySwitch.sendTriState("00000FFF0FF0");

    delay(1000);
    return 0;
}
