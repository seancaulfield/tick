/*
 * charlieplexer.ino
 * 
 * Program to test the charlieplexing of this board.
 *
 * Author: Sean Caulfield <sean@yak.net>
 * License: GPLv2
 *
 */

#include <Arduino.h>
#include "usec.h"
//#include <Wire.h>
//#include <RTClib.h>
//#include <DS3231.h>
//#include <Time.h>
//#include <Timezone.h>
#include "charlie.h"

const int PINS = 15;
const int SEGMENTS = 8;
const int DIGITS = 12;

const int all_pins[PINS]     = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, A0, A1, A2 };

const int display0[SEGMENTS] = {  9, 10, 11, 12, 13, A0, A1, A2 };
const int display1[SEGMENTS] = { A2,  2,  3,  4,  5, 13, A0, A1 };
const int display2[SEGMENTS] = {  2,  3,  4,  5,  6, A0, A1, A2 };
const int cathodes[DIGITS] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };

const int SERIAL_BAUD = 9600;

const int DELAY = 50;
const usec LINGER = 150000;

void set_h(int pin) {
  digitalWrite(pin, HIGH);
  pinMode(pin, OUTPUT);
}

void set_l(int pin) {
  digitalWrite(pin, LOW);
  pinMode(pin, OUTPUT);
}

void set_z(int pin) {
  digitalWrite(pin, HIGH);
  pinMode(pin, INPUT);
}

// Get array of segment anodes
const int *getAnodes(int digit) {
  if (digit >= 0 && digit < 4) {
    return display0;
  } else if (digit >= 4 && digit < 8) {
    return display1;
  } else {
    return display2;
  }
}

// Perform a single linger cycle on a single segment
void light_cycle(int pin) {
  usec start_time = micros();
  while (USEC_DIFF(micros(), start_time) < LINGER) {
    set_l(pin);
    delay(DELAY);
    set_z(pin);
    delay(DELAY);
  }
}

void all_off() {
  for (int i=0; i<PINS; i++) {
    set_z(all_pins[i]);
  }
}

void setup() {
  all_off();
}

void loop() {

  for (int digit=0; digit<DIGITS; digit++) {
    const int *segment_anodes = getAnodes(digit);

    for (int s=0; s<SEGMENTS; s++) {

      // Set anode to high to source current
      set_h(segment_anodes[s]);

      // Pull digit cathode low for a linger cycle
      light_cycle(cathodes[digit]);

      // Put anode back into high-Z state
      set_z(segment_anodes[s]);

    }

  }

}
