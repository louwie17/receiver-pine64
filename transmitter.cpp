/*
 * http://joey.hazlett.us/pine64/pine64_pins.html
 */
#include "wiringPi.h"
#include "RCSwitch.h"
#include <stdio.h>
#include <stdlib.h>
#include <bitset>
#include <iostream>
#include <sstream>
#include <string>
using namespace std;

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
    //mySwitch.setRepeatTransmit(5);
    mySwitch.enableTransmit(TRANSMITTER_PIN);

    string myString = "Hello World";
    string send = "";
    for (std::size_t i = 0; i < myString.size(); ++i) {
        ostringstream stream;
        stream << bitset<8>(myString.c_str()[i]) << endl;
        send.append(stream.str());
    }
    mySwitch.send(send.c_str());
    delay(1000);

    return 0;
}
