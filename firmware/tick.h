#ifndef __TICK_H
#define __TICK_H

#if ARDUINO < 100
#include <WProgram.h>
#else
#include <Arduino.h>
#endif

#ifdef __AVR_ATtiny85__
#include <TinyWireM.h>
#define Wire TinyWireM
#else
#include <Wire.h>
#endif

#include <RTCLib/DS3231.h>
#include <Time.h>
#include <Timezone.h>

#endif // __TICK_H
