/*
 * usec.h - Functions to help with comparing microsecond times, since I find
 * myself using them a lot to avoid the hazards of functions like delay().
 */

#ifndef USEC_H
#define USEC_H

#include<limits.h>

typedef unsigned long usec;

// Overflow safe microseconds comparator
#define USEC_DIFF(x, y) \
  (((x) > (y)) ? ((x) - (y)) : ((x) + (ULONG_MAX - (y))))

#endif
