/*
 * The timer module handles the timingof the program, including generating a
 * random time in milliseconds.
 */
#ifndef TIMER_H  // Inclusion guard.
#define TIMER_H  // Inclusion guard.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void Timer_sleepForMs(long long delayInMs);
long long Timer_getTimeInMs(void);


#endif
