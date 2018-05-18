// License MIT
// Compatibility layer done by Kamil Trzcinski <ayufan@ayufan.eu>

#include "c_gpio.h"
#include <sys/time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#ifndef	TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(!TRUE)
#endif
#define	WPI_FATAL	(1==1)
#define WPI_ALMOST  (1==2)

#define UNU __attribute__((unused))
// Interrupt levels

#ifdef DEBUG
#define DEBUG_PRINT(fmt, args...)    fprintf(stderr, fmt, ## args)
#else
#define DEBUG_PRINT(fmt, args...)    /* Don't do anything in release builds */
#endif

#define	INT_EDGE_SETUP		0
#define	INT_EDGE_FALLING	1
#define	INT_EDGE_RISING		2
#define	INT_EDGE_BOTH		3

extern int wiringPiFailure (int fatal, const char *message, ...);

extern int wiringPiSetup();
void wiringPiCleanup();
int digitalRead(int gpio);
void digitalWrite(int gpio, int value);
void pinMode(int gpio, int direction);
void initialiseEpoch (void);
void delayMicrosecondsHard (unsigned int howLong);
void delayMicroseconds (unsigned int howLong);
void delay (unsigned int howLong);
unsigned int millis (void);
unsigned int micros (void);

// Interrupts
//	(Also Pi hardware specific)

extern int  wiringPiISR         (int pin, int mode, void (*function)(void)) ;

extern int piHiPri (const int pri) ;
