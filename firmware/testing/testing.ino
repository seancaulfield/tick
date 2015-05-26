/*
 * testing.ino
 * 
 * Program to test shit. I hate my existence.
 *
 * Author: Sean Caulfield <sean@yak.net>
 * License: GPLv2
 *
 */

#include <Arduino.h>
#include "usec.h"

const int DIGITS   = 12;
const int SEGMENTS = 8;

int CATHODES[DIGITS] = { 5, 4, 3, 2, 6, 7, 8, 9, 10, 11, 12, 13 };
int ANODES[DIGITS][SEGMENTS] = {
  {  9, 10, 11, 12, 13, A0, A1, A2 },
  {  9, 10, 11, 12, 13, A0, A1, A2 },
  {  9, 10, 11, 12, 13, A0, A1, A2 },
  {  9, 10, 11, 12, 13, A0, A1, A2 },
  { A2,  2,  3,  4,  5, 13, A0, A1 },
  { A2,  2,  3,  4,  5, 13, A0, A1 },
  { A2,  2,  3,  4,  5, 13, A0, A1 },
  { A2,  2,  3,  4,  5, 13, A0, A1 },
  {  2,  3,  4,  5,  6, A0, A1, A2 },
  {  2,  3,  4,  5,  6, A0, A1, A2 },
  {  2,  3,  4,  5,  6, A0, A1, A2 },
  {  2,  3,  4,  5,  6, A0, A1, A2 }
};

const usec PULSE_ON  = 1000;
const usec PULSE_OFF = 80;
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
  pinMode(pin, INPUT);
}

void setup() {
  set_z(2);
  set_z(3);
  set_z(4);
  set_z(5);
  set_z(6);
  set_z(7);
  set_z(8);
  set_z(9);
  set_z(10);
  set_z(11);
  set_z(12);
  set_z(13);
  set_z(A0);
  set_z(A1);
  set_z(A2);
}

void loop() {
  for (int digit=0; digit<1; digit++) {
    for (int seg=0; seg<SEGMENTS; seg++) {
      usec start_time = micros();
      set_l(CATHODES[digit]);
      while (USEC_DIFF(micros(), start_time) < LINGER) {
        set_h(ANODES[digit][seg]);
        delayMicroseconds(PULSE_ON);
        set_z(ANODES[digit][seg]);
        delayMicroseconds(PULSE_OFF);
      }
    }
  }
}
