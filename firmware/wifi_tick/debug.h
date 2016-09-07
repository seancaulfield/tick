/*
 * debug.h - Debugging macros
 *
 */

#ifndef __DEBUG_H
#define __DEBUG_H

#define DEBUG 0
#define SERIAL_BAUD 115200

#if (DEBUG)
#define DPRINT(...) Serial.print(__VA_ARGS__)
#define DPRINTLN(...) Serial.println(__VA_ARGS__)
#define DFLUSH(...) Serial.flush(__VA_ARGS__)
#else
#define DPRINT(...)
#define DPRINTLN(...)
#define DFLUSH(...)
#endif

#endif
