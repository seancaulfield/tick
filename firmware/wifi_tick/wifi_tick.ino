/*
 * wifi_tick.ino - NTP-ified T.I.C.K.
 *
 * Author:  Sean Caulfield <sean@yak.net>
 * License: GPLv2
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <SPI.h>
#include <Wire.h>

#include <LedControl.h>
#include <RTClib.h>
#include <DS3231.h>
#include <Time.h>
#include <Timezone.h>
#include <NTPClient.h>

#include <string.h>

#include "debug.h"
#include "config.h"

//
// Display pin assignments & settings
//

#define DISP_DATA      13
#define DISP_CLCK      14
#define DISP_LOAD      12
#define DISP_NUM        2
#define BRIGHT_LARGE   15
#define BRIGHT_SMALL    5

// Maximum diff between NTP and RTC before adjusting the RTC to match
#define MAX_TIME_DIFF   5

inline uint32_t usec_diff(uint32_t a, uint32_t b) {
  if (a > b) {
    return a - b;
  } else {
    return b - a;
  }
}

//
// Global controls/vars
//

ESP8266WiFiMulti wifi;
NTPClient ntpclient;
LedControl display = LedControl(DISP_DATA, DISP_CLCK, DISP_LOAD, DISP_NUM);
RTC_DS3231 rtc;

uint32_t ntp_time = 0;
uint32_t rtc_time = 0;
DateTime dt_utc;
DateTime dt_local;

//
// Timezones - These are hard coded for now, but plan is to try to support
// actual TZ files, and hopefully automatically download & update them.
//

TimeChangeRule est = {"EST", First,  Sun, Nov, 2, -5*60};
TimeChangeRule edt = {"EDT", Second, Sun, Mar, 2, -4*60};
Timezone tz = Timezone(edt, est);

//
// SETUP() - This is done once at startup.
//

void setup() {

#if (DEBUG)
  delay(200);
  Serial.begin(SERIAL_BAUD);
  delay(10);
#endif

  //
  // Setup large digit display. Have to set the scan limit as it's only 4
  // digits (but we'll be pushing the chip a bit because each segment is
  // actually 2 LEDs).
  //

  display.shutdown(0, false);
  display.setScanLimit(0, 4);
  display.setIntensity(0, BRIGHT_LARGE);
  display.clearDisplay(0);

  //
  // Setup small digit displays.
  //

  display.shutdown(1, false);
  display.setIntensity(1, BRIGHT_SMALL);
  display.clearDisplay(1);

  //
  // Setup RTC
  //
  Wire.begin();
  if (rtc.begin() && !rtc.isrunning()) {
    rtc.enable();
  }

  //
  // Connect to Wifi
  //

  DPRINT("Connecting to wifi"); DFLUSH();
  wifi.addAP(MY_WIFI_SSID, MY_WIFI_PASS);
  while (wifi.run() != WL_CONNECTED) {
    DPRINT("."); DFLUSH();
    delay(100);
  }
  DPRINTLN("\nSuccess!"); DFLUSH();

}

//
// LOOP() - This is done lots, like your mom.
//

void loop() {

  // Has its own internal check to only poll every 60s

  ntpclient.update();

  // Get current time

  ntp_time = ntpclient.getRawTime();
  rtc_time = rtc.now().unixtime();

  // Pick saner of two time sources
  if (ntpclient.isValidYet()) {
    dt_utc.setTime(ntp_time);
    dt_local.setTime(tz.toLocal(ntp_time));
  } else {
    dt_utc.setTime(rtc_time);
    dt_local.setTime(tz.toLocal(rtc_time));
  }
  DPRINTLN(dt_utc.iso8601());
  DPRINTLN(dt_local.iso8601());

  // Set RTC if it looks like it's not been set
  // TODO Probab should keep track of how many times this has had to be done in
  // "recent" memory (ugh, logging to EEPROM, probs) and warn user if we've had
  // to do it a lot, because the RTC battery is probably dead/gone. Hmm. Is
  // there something in the DS3231 that would just TELL me the battery voltage?
  // That'd be right handy... :P
  if (ntpclient.isValidYet() && usec_diff(ntp_time, rtc_time) > MAX_TIME_DIFF) {
    rtc.adjust(dt_utc);
  }

  // Refresh large display (HH:MM). Try to emulate colon by setting decimal on
  // middle digits.

  display.clearDisplay(0);
  if (dt_local.hour() >= 10)
    display.setDigit(0, 0, dt_local.hour() / 10, false);
  display.setDigit(0, 1, dt_local.hour() % 10, true);
  display.setDigit(0, 2, dt_local.minute() / 10, true, true);
  display.setDigit(0, 3, dt_local.minute() % 10, false, true);

  // Refresh small displays (YYYY MM.DD)

  display.clearDisplay(1);
  display.setDigit(1, 0, dt_local.month() / 10, false);
  display.setDigit(1, 1, dt_local.month() % 10, true);
  display.setDigit(1, 2, dt_local.day() / 10, false);
  display.setDigit(1, 3, dt_local.day() % 10, false);
  display.setDigit(1, 4, dt_local.year() / 1000, false);
  display.setDigit(1, 5, (dt_local.year() / 100) % 10, false);
  display.setDigit(1, 6, (dt_local.year() / 10) % 10, false);
  display.setDigit(1, 7, dt_local.year() % 10, true);

  delay(1000);

}
