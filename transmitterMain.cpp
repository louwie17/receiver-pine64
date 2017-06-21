/*
 * http://joey.hazlett.us/pine64/pine64_pins.html
 */
#include "Transmitter.h"
#include <stdio.h>
#include <stdlib.h>

Transmitter myTransmitter;

int TRANSMITTER_PIN = 71;
// The char to transmit
char * strToSend = "he";

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

    myTransmitter = Transmitter();
    myTransmitter.setDelay(1);
    myTransmitter.setPin(TRANSMITTER_PIN);

    myTransmitter.sendString(strToSend);
    return 0;
}
