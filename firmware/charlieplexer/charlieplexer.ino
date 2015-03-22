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
#include <Wire.h>
#include <RTClib.h>
#include <DS3231.h>
#include <Time.h>
#include <Timezone.h>

void all_off() {
  pinMode(2, INPUT_PULLUP);  digitalWrite(2,  HIGH);
  pinMode(3, INPUT_PULLUP);  digitalWrite(3,  HIGH);
  pinMode(4, INPUT_PULLUP);  digitalWrite(4,  HIGH);
  pinMode(5, INPUT_PULLUP);  digitalWrite(5,  HIGH);
  pinMode(6, INPUT_PULLUP);  digitalWrite(6,  HIGH);
  pinMode(7, INPUT_PULLUP);  digitalWrite(7,  HIGH);
  pinMode(8, INPUT_PULLUP);  digitalWrite(8,  HIGH);
  pinMode(9, INPUT_PULLUP);  digitalWrite(9,  HIGH);
  pinMode(10, INPUT_PULLUP); digitalWrite(10, HIGH);
  pinMode(11, INPUT_PULLUP); digitalWrite(11, HIGH);
  pinMode(12, INPUT_PULLUP); digitalWrite(12, HIGH);
  pinMode(13, INPUT_PULLUP); digitalWrite(13, HIGH);
  pinMode(A0, INPUT_PULLUP);  digitalWrite(A0,  HIGH);
  pinMode(A1, INPUT_PULLUP);  digitalWrite(A1,  HIGH);
  pinMode(A2, INPUT_PULLUP);  digitalWrite(A2,  HIGH);
}

void light(int anode, int cathode) {
  pinMode(cathode, INPUT);  digitalWrite(cathode, LOW);
  pinMode(anode,   OUTPUT); digitalWrite(anode,   HIGH);
}

void setup() {
  all_off();
}

void loop() {
}
