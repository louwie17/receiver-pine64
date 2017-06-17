/*
 * http://joey.hazlett.us/pine64/pine64_pins.html
 * Interruptable pins I know so far:
 * 3 -> 227
 * 5 -> 226
 */
#include "wiringPi.h"
#include "RCSwitch.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#define MAX_TIMINGS	  86


RCSwitch mySwitch = RCSwitch();

int RECEIVER_PIN = 71;

static const char* bin2tristate(const char* bin);
static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength);

void output(unsigned long decimal, unsigned int length, unsigned int delay, unsigned int* raw, unsigned int protocol) {

  if (decimal == 0) {
    printf("Unknown encoding.");
  } else {
    const char* b = dec2binWzerofill(decimal, length);
    printf("Decimal: %lu", decimal);
    printf(" (%d Bit) Binary: %s", length, b);
    printf(" Tri-State: %s", bin2tristate( b));
    printf(" PulseLength: %d", delay);
    printf(" microseconds Protocol: %d\n", protocol);
  }
  
  printf("Raw data: ");
  for (unsigned int i=0; i<= length*2; i++) {
    printf("%d,", raw[i]);
  }
  printf("\n\n");
}

static const char* bin2tristate(const char* bin) {
  static char returnValue[50];
  int pos = 0;
  int pos2 = 0;
  while (bin[pos]!='\0' && bin[pos+1]!='\0') {
    if (bin[pos]=='0' && bin[pos+1]=='0') {
      returnValue[pos2] = '0';
    } else if (bin[pos]=='1' && bin[pos+1]=='1') {
      returnValue[pos2] = '1';
    } else if (bin[pos]=='0' && bin[pos+1]=='1') {
      returnValue[pos2] = 'F';
    } else {
      return "not applicable";
    }
    pos = pos+2;
    pos2++;
  }
  returnValue[pos2] = '\0';
  return returnValue;
}

static char * dec2binWzerofill(unsigned long Dec, unsigned int bitLength) {
  static char bin[64];
  unsigned int i=0;

  while (Dec > 0) {
    bin[32+i++] = ((Dec & 1) > 0) ? '1' : '0';
    Dec = Dec >> 1;
  }

  for (unsigned int j = 0; j< bitLength; j++) {
    if (j >= bitLength - i) {
      bin[j] = bin[ 31 + i - (j - (bitLength - i)) ];
    } else {
      bin[j] = '0';
    }
  }
  bin[bitLength] = '\0';

  return bin;
}

void cleanupReceiver(int dummy) {
  wiringPiCleanup();
  exit(1);
}

int main(int argc, char *argv[])
{

    while (true) {
        switch (getopt(argc, argv, "d:r:p:s:")) {
            case -1:
                goto done;

            case 'p':
                RECEIVER_PIN = atoi(optarg);
                break;

            default: /* '?' */
                break;
        }
    }

    done:

    if (wiringPiSetup () == -1) return 1;
    printf("Using pin: %d\n", RECEIVER_PIN);
    RCSwitch mySwitch = RCSwitch();
    mySwitch.enableReceive(0, RECEIVER_PIN);

  signal(SIGINT, cleanupReceiver);

    while (1) {
        if (mySwitch.available()) {
            output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
            mySwitch.resetAvailable();
        }
    }
    return 0;
}
