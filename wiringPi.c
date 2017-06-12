// License MIT
// Compatibility layer done by Kamil Trzcinski <ayufan@ayufan.eu>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <poll.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>

#include "wiringPi.h"

//
// For Pine A64/A64+ Board
//
// pinToGpio:
//
const int pinToGpioPineA64[41] = {	// BCM ModE
     -1,  -1, 227, 226, 362, 229, 230, 231, // 0..7
     67,  65,  64,  66,  68,  69,  32,  33, // 8..15
     70,  71,  72,  73,  74,  75,  76,  77, // 16..23
     78,  79,  80, 233,  -1,  -1,  -1,  -1, // 24..31
// Padding:
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, // ... 40
};

//
// physToGpio:
//
const int physToGpioPineA64[41] =	// BOARD MODE
{
  -1,       // 0
  -1,  -1,	// 1, 2
 227,  -1,
 226,  -1,
 362,  32,
  -1,  33,
  71,  72,
 233,  -1,
  76,  77,
  -1,  78,
  64,  -1,
  65,  79,
  66,  67,
  -1, 231,	// 25, 26

 361, 360,
 229,  -1,
 230,  68,
  69,  -1,
  73,  70,
  80,  74,
  -1,  75, // 39, 40
};

int get_bcm_number(int gpio) {
  if (gpio < 27) { // Already bcm
    return gpio;
  }

  int index = 0;

  while ( index < sizeof(pinToGpioPineA64) && pinToGpioPineA64[index] != gpio ) ++index;

  return ( index == sizeof(pinToGpioPineA64) ? -1 : index );
}

int get_gpio_number(int gpio)
{
    if (gpio > 1 && gpio < 27) {
        // Is BCM GPIO
        return pinToGpioPineA64[gpio];
    } else if (gpio > 31 && gpio < 363) {
        // Is Pine GPIO most likely
        return gpio;
    }
    return -1;
}

static uint64_t epochMilli, epochMicro;

int wiringPiSetup()
{
  initialiseEpoch();
  return setup();
}

int digitalRead(int gpio)
{
  return input_gpio(gpio);
}

void digitalWrite(int gpio, int value)
{
  output_gpio(gpio, value);
}

void pinMode(int gpio, int direction)
{
  setup_gpio(gpio, direction, PUD_UP);
}

//
// The functions belowe are copied from wiringPi
//

void initialiseEpoch (void)
{
  struct timeval tv ;

  gettimeofday (&tv, NULL) ;
  epochMilli = (uint64_t)tv.tv_sec * (uint64_t)1000    + (uint64_t)(tv.tv_usec / 1000) ;
  epochMicro = (uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)(tv.tv_usec) ;
}

void delayMicrosecondsHard (unsigned int howLong)
{
  struct timeval tNow, tLong, tEnd ;

  gettimeofday (&tNow, NULL) ;
  tLong.tv_sec  = howLong / 1000000 ;
  tLong.tv_usec = howLong % 1000000 ;
  timeradd (&tNow, &tLong, &tEnd) ;

  while (timercmp (&tNow, &tEnd, <))
    gettimeofday (&tNow, NULL) ;
}

void delayMicroseconds (unsigned int howLong)
{
  struct timespec sleeper ;
  unsigned int uSecs = howLong % 1000000 ;
  unsigned int wSecs = howLong / 1000000 ;

  /**/ if (howLong ==   0)
    return ;
  else if (howLong  < 100)
    delayMicrosecondsHard (howLong) ;
  else
  {
    sleeper.tv_sec  = wSecs ;
    sleeper.tv_nsec = (long)(uSecs * 1000L) ;
    nanosleep (&sleeper, NULL) ;
  }
}

void delay (unsigned int howLong)
{
  struct timespec sleeper, dummy ;

  sleeper.tv_sec  = (time_t)(howLong / 1000) ;
  sleeper.tv_nsec = (long)(howLong % 1000) * 1000000 ;

  nanosleep (&sleeper, &dummy) ;
}

unsigned int millis (void)
{
  struct timeval tv ;
  uint64_t now ;

  gettimeofday (&tv, NULL) ;
  now  = (uint64_t)tv.tv_sec * (uint64_t)1000 + (uint64_t)(tv.tv_usec / 1000) ;

  return (uint32_t)(now - epochMilli) ;
}

unsigned int micros (void)
{
  struct timeval tv ;
  uint64_t now ;

  gettimeofday (&tv, NULL) ;
  now  = (uint64_t)tv.tv_sec * (uint64_t)1000000 + (uint64_t)tv.tv_usec ;

  return (uint32_t)(now - epochMicro) ;
}

/*
 * wiringPiFailure:
 *	Fail. Or not.
 *********************************************************************************
 */

int wiringPiReturnCodes = FALSE ;
int wiringPiFailure (int fatal, const char *message, ...)
{
  va_list argp ;
  char buffer [1024] ;

  if (!fatal && wiringPiReturnCodes)
    return -1 ;

  va_start (argp, message) ;
    vsnprintf (buffer, 1023, message, argp) ;
  va_end (argp) ;

  fprintf (stderr, "%s", buffer) ;
  exit (EXIT_FAILURE) ;

  return 0 ;
}

// ===================== Listener stuff


static int sysFds [32] =
{
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static void (*isrFunctions [64])(void) ;
static volatile int    pinPass = -1 ;
static volatile int    allowCheckForInterrupt = 1;
static pthread_mutex_t pinMutex ;

void wiringPiCleanup()
{
  cleanup();
  allowCheckForInterrupt = 0;
  int fd, i;
  int count = sizeof(sysFds);
  for (i = 0 ; i < count ; ++i) {
    if ((fd = sysFds[count]) != -1) {
      printf("cleaned up pin: %d\n", count);
      close(fd);
    }
  }
}

/*
 * waitForInterrupt:
 *	Pi Specific.
 *	Wait for Interrupt on a GPIO pin.
 *	This is actually done via the /sys/class/gpio interface regardless of
 *	the wiringPi access mode in-use. Maybe sometime it might get a better
 *	way for a bit more efficiency.
 *********************************************************************************
 */

int waitForInterrupt (int pin, int mS)
{
  int fd, x ;
  uint8_t c ;
  struct pollfd polls ;

  if ((fd = sysFds [pin]) == -1)
    return -2 ;

// Setup poll structure

  polls.fd     = fd ;
  polls.events = POLLPRI | POLLERR ;

// Wait for it ...

  //printf("polling\n");
  x = poll (&polls, 1, mS) ;

// If no error, do a dummy read to clear the interrupt
//	A one character read appars to be enough.

  //printf("x: %d", x);
  if (x > 0)
  {
    lseek (fd, 0, SEEK_SET) ;	// Rewind
    (void)read (fd, &c, 1) ;	// Read & clear
  }

  return x ;
}

/*
 * interruptHandler:
 *	This is a thread and gets started to wait for the interrupt we're
 *	hoping to catch. It will call the user-function when the interrupt
 *	fires.
 *********************************************************************************
 */
static void *interruptHandler (UNU void *arg)
{
  int myPin ;

  (void)piHiPri (55) ;	// Only effective if we run as root

  myPin   = pinPass;
  pinPass = -1 ;

  for (allowCheckForInterrupt) {
    if (waitForInterrupt (myPin, -1) > 0) {
      //printf("interrupt\n");
      isrFunctions [myPin] () ;
    }
  }
  pthread_exit(NULL);
}

int gpio_set_direction(unsigned int gpio, unsigned int in_flag)
{
    int retry;
    struct timespec delay;
    int fd;
    char filename[34];

    snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/direction", gpio);
    delay.tv_sec = 0;
    delay.tv_nsec = 10000000L; // 10ms
    for (retry=0; retry<100; retry++) {
        if ((fd = open(filename, O_WRONLY)) >= 0)
            break;
        nanosleep(&delay, NULL);
    }
    if (retry >= 100)
        return -1;

    if (in_flag)
        write(fd, "in", 3);
    else
        write(fd, "out", 4);

    close(fd);
    return 0;
}

int gpio_set_edge(unsigned int gpio, unsigned int mode)
{
  const char *modeS ;
  pid_t pid ;

  if (mode != INT_EDGE_SETUP)
  {
    if (mode == INT_EDGE_FALLING) {
      modeS = "falling" ;
    } else if (mode == INT_EDGE_RISING) {
      modeS = "rising" ;
    } else {
      modeS = "both" ;
    }

    if ((pid = fork ()) < 0)	// Fail
      return wiringPiFailure (WPI_FATAL, "wiringPiISR: fork failed: %s\n", strerror (errno)) ;

    if (pid == 0)	// Child, exec
    {
        // Setting edge
      int fd;
      char filename[29];

      printf("Setting edge\n");
      snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/edge", gpio, strerror (errno));

      if ((fd = open(filename, O_WRONLY)) < 0) {
        return wiringPiFailure(WPI_FATAL, "wiringPiISR: open edge failed.\n");
      }

      write(fd, modeS, strlen(modeS) + 1);
      close(fd);
      printf("finished setting edge\n");
    }
    else		// Parent, wait
      wait (NULL) ;
  }
}


/*
 * wiringPiISR:
 *	Pi Specific.
 *	Take the details and create an interrupt handler that will do a call-
 *	back to the user supplied function.
 *********************************************************************************
 */

int wiringPiISR (int pin, int mode, void (*function)(void))
{
  pthread_t threadId ;
  char fName   [64] ;
  char  pinS [8] ;
  int   count, i, result;
  char  c ;
  int bouncetime = 100;
  int gpioPin = get_gpio_number(pin);
  int bmcPin = get_bcm_number(pin);

  char gpioDirName[64];
  snprintf(gpioDirName, sizeof(gpioDirName), "/sys/class/gpio/gpio%d", gpioPin);

  printf("dirName: %s \n", gpioDirName);
  if (access (gpioDirName, X_OK) == -1) {
      // gpio export
      int fd, len;
      char str_gpio[4];

      printf("Exporting\n");
      if ((fd = open("/sys/class/gpio/export", O_WRONLY)) < 0)
          return -1;

      printf("Finished exporting");
      len = snprintf(str_gpio, sizeof(str_gpio), "%d", gpioPin);
      write(fd, str_gpio, len);
      close(fd);
  }
  // set direction
  if (gpio_set_direction(gpioPin, 1) == -1) {
      return wiringPiFailure (WPI_FATAL, "wiringPiISR: direction failed\n", strerror (errno)) ;
  }
  printf("finished setting direction\n");
// Now export the pin and set the right edge
//	We're going to use the gpio program to do this, so it assumes
//	a full installation of wiringPi. It's a bit 'clunky', but it
//	is a way that will work when we're running in "Sys" mode, as
//	a non-root user. (without sudo)

  gpio_set_edge(gpioPin, mode);

  if (sysFds [bcmPin] == -1)
  {
    sprintf (fName, "/sys/class/gpio/gpio%d/value", gpioPin) ;
    printf("opening %s\n", fName);
    if ((sysFds [bcmPin] = open (fName, O_RDWR)) < 0) {
      printf("failed open");
      return wiringPiFailure (WPI_FATAL, "wiringPiISR: unable to open %s: %s\n", fName, strerror (errno)) ;
    }
  }

// Clear any initial pending interrupt

  ioctl (sysFds [bcmPin], FIONREAD, &count) ;
  printf("ioctl count: %d", count);
  for (i = 0 ; i < count ; ++i) {
    read (sysFds [bcmPin], &c, 1);
  }

  isrFunctions [bcmPin] = function ;

  pthread_mutex_lock (&pinMutex) ;
    pinPass = bcmPin ;
    pthread_create (&threadId, NULL, interruptHandler, NULL) ;
    while (pinPass != -1) {
      delay (1) ;
    }
  pthread_mutex_unlock (&pinMutex) ;
  return 0;
}
