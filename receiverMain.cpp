/*
 * http://joey.hazlett.us/pine64/pine64_pins.html
 * Interruptable pins I know so far:
 * 3 -> 227
 * 5 -> 226
 */
#include "Receiver.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

Receiver myReceiver;

int RECEIVER_PIN = 71;

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

  myReceiver = Receiver();
  myReceiver.setPin(RECEIVER_PIN);
  myReceiver.setOID(288);

  signal(SIGINT, cleanupReceiver);

  while(1){
      //This will fulfill 'example' with the received string
      char * example = myReceiver.receiveData();
      if(myReceiver.getIsString()){
          printf("Message: %s\n", example);
      }else if(myReceiver.getIsCommand()){
          printf("Commande: %s\n", example);
      }
  }

    return 0;
}
