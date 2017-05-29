/*
 * wifi_tick.ino - NTP-ified T.I.C.K.
 *
 * Author:  Sean Caulfield <sean@yak.net>
 * License: GPLv2
 *
 */

#include <SPI.h>
#include <Wire.h>

#include <LedControl.h>
#include <RTClib.h>
#include <DS3231.h>
#include <Time.h>
#include <Timezone.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ESP8266WebServer.h>

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

// Admin web server port
#define HTTP_PORT 80

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

WiFiUDP ntpUDP;
NTPClient ntpclient(ntpUDP);
ESP8266WebServer server(HTTP_PORT);
LedControl display = LedControl(DISP_DATA, DISP_CLCK, DISP_LOAD, DISP_NUM);
RTC_DS3231 rtc;

uint32_t ntp_time = 0;
uint32_t rtc_time = 0;
DateTime dt_utc;
DateTime dt_local;

boolean firstConnected = false;
boolean httpServerSetup = false;

//
// Timezones - These are hard coded for now, but plan is to try to support
// actual TZ files, and hopefully automatically download & update them.
//

TimeChangeRule est = {"EST", First,  Sun, Nov, 2, -5*60};
TimeChangeRule edt = {"EDT", Second, Sun, Mar, 2, -4*60};
Timezone tz = Timezone(edt, est);

////////////////////////////////////////////////////////////////////////////////

//
// Handle the network stuff so it can be disabled in case wifi isn't available.
//
void handle_net() {

  // Get current time from NTP. Has its own internal check to only poll every
  // 60s so we're not blasting the NTP pool servers.
  ntpclient.update();

  // Get current time
  ntp_time = ntpclient.getEpochTime();

  // Handle any web serving we're supposed to be doing
  if (httpServerSetup) {
    server.handleClient();
  }

}

//
// Refresh global time variable from whichever source we've got.
//
void refresh_time() {

  // Get current time from RTC
  rtc_time = rtc.now().unixtime();

  // Update global local & UTC datetime objects with NTP if it's available and
  // the RTC otherwise.
  if (ntpclient.isValidYet()) {
    dt_utc.setTime(ntp_time);
    dt_local.setTime(tz.toLocal(ntp_time));
  } else {
    dt_utc.setTime(rtc_time);
    dt_local.setTime(tz.toLocal(rtc_time));
  }

  // Set RTC if it looks like it's not been set and set it if not.
  if (ntpclient.isValidYet() && usec_diff(ntp_time, rtc_time) > MAX_TIME_DIFF) {
    rtc.adjust(dt_utc);
  }

  //DPRINTLN(dt_utc.iso8601());
  //DPRINTLN(dt_local.iso8601());

}

//
// Refresh display with current local time (from global var).
//
void refresh_display() {

  // Refresh large display (HH:MM). Try to emulate colon by setting decimal on
  // middle digits.
  display.clearDisplay(0);
  if (dt_local.hour() >= 10)
    display.setDigit(0, 0, dt_local.hour() / 10, false);
  display.setDigit(0, 1, dt_local.hour() % 10, true);
  display.setDigit(0, 2, dt_local.minute() / 10, true, true);
  display.setDigit(0, 3, dt_local.minute() % 10, false, true);

  // Refresh small displays (YYYY MM DD)
  display.clearDisplay(1);
  display.setDigit(1, 0, dt_local.month() / 10, false);
  display.setDigit(1, 1, dt_local.month() % 10, true);
  display.setDigit(1, 2, dt_local.day() / 10, false);
  display.setDigit(1, 3, dt_local.day() % 10, false);
  display.setDigit(1, 4, dt_local.year() / 1000, false);
  display.setDigit(1, 5, (dt_local.year() / 100) % 10, false);
  display.setDigit(1, 6, (dt_local.year() / 10) % 10, false);
  display.setDigit(1, 7, dt_local.year() % 10, true);

}

//
// Setup routing to callbacks for web server (but only if we've got wifi)
//
void setup_server() {
  // TODO
}

////////////////////////////////////////////////////////////////////////////////

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
  // Connect to Wifi. Not bothering to check if it's ready as that's done in
  // the main loop (as the gate for doing network-related stuff).
  //

  DPRINTLN("Connecting to wifi"); DFLUSH();
  WiFi.begin(MY_WIFI_SSID, MY_WIFI_PASS);

}

//
// LOOP() - This is done lots, like your mom.
//

void loop() {

  // Update time & display
  refresh_time();
  refresh_display();

  // Check if Wifi is connected and if so, handle networking stuff
  if (WiFi.status() == WL_CONNECTED) {
    if (!firstConnected) {
      firstConnected = true;
      DPRINT("Connected to wifi! IP ");
      DPRINTLN(WiFi.localIP());
      ntpclient.begin();
      boolean result = ntpclient.forceUpdate();
      DPRINT("NTP result = ");
      DPRINTLN(result);
      DPRINTLN(ntp_time);
      setup_server();
    }
    handle_net();
  }

  delay(1000);
  //yield();

}
