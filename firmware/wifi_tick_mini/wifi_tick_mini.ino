/*
 * esp8266_ohai.ino - Something of a WiFi Hello World.
 *
 */

#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <FS.h>

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <RTClib.h>
#include <DS3231.h>
#include <Time.h>
#include <Timezone.h>
#include <NTPClient.h>

#include <string.h>

#include "debug.h"
#include "wifi_setup.h"

// Pin assignments

#define DISP_ADDR_START 0x70
#define DISP_ADDR_END   0x77
#define DISP_BRIGHTNESS 15   //0-15

//
// TODO Define these for yourself!
//
const char *MY_WIFI_AP_NAME = "<SSID>";
const char *MY_WIFI_AP_KEY  = "<KEY>";

//
// Global controls/vars
//

const size_t NUM_DISPLAYS = 1;
Adafruit_7segment displays[NUM_DISPLAYS] = {
  Adafruit_7segment(),
  //Adafruit_7segment(),
  //Adafruit_7segment(),
  //Adafruit_7segment(),
  //Adafruit_7segment(),
  //Adafruit_7segment(),
  //Adafruit_7segment(),
  //Adafruit_7segment()
};

RTC_DS3231 rtc = RTC_DS3231();
NTPClient ntpclient;

boolean led_state = false;

void setup_serial() {
  delay(200);
  Serial.begin(SERIAL_BAUD);
  delay(10);
}

void displayTime(DateTime *dt) {

  for (byte i; i<NUM_DISPLAYS; i++) {
    byte hour = (dt->hour() + i) % 24;
    displays[i].clear();
    if (hour >= 20) {
      displays[i].writeDigitNum(0, 2, false);
    } else if (hour >= 10) {
      displays[i].writeDigitNum(0, 1, false);
    }
    displays[i].writeDigitNum(1, hour % 10, false);
    displays[i].writeDigitRaw(2, 0x02); //colon
    displays[i].writeDigitNum(3, dt->minute() / 10, false);
    displays[i].writeDigitNum(4, dt->minute() % 10, false);
    displays[i].writeDisplay();
  }

}

void setup() {

#ifdef DEBUG
  setup_serial();
#endif

  //
  // Start I2C & RTC
  //
  Wire.begin();
  rtc.begin();
  rtc.clearControlRegisters();

  //
  // Setup display(s)
  //
  for (byte i=0; i<NUM_DISPLAYS; i++) {
    displays[i].begin(DISP_ADDR_START + i);
  }
  for (byte i=0; i<NUM_DISPLAYS; i++) {
    displays[i].setBrightness(DISP_BRIGHTNESS);
    displays[i].clear();
    displays[i].writeDigitRaw(0, 8);
    displays[i].writeDigitRaw(1, 8);
    displays[i].writeDigitRaw(2, 2);
    displays[i].writeDigitRaw(3, 8);
    displays[i].writeDigitRaw(4, 8);
    displays[i].writeDisplay();
  }

  //
  // Mount config filesystem
  //
  if (!SPIFFS.begin()) {
    DPRINTLN("Failed to mount config fs!");
    return;
  }

  //
  // Config probably doesn't exist, so try to save it to SPIFFS
  //

  if (!setup_wifi()) {

    size_t len_name = strlen(MY_WIFI_AP_NAME);
    wifi_ap_name = new char[len_name+1];
    strncpy(wifi_ap_name, MY_WIFI_AP_NAME, len_name);
    if (save_ap_name()) {
      DPRINTLN("Saved wifi_ap_name");
    } else {
      DPRINTLN("Failed to save wifi_ap_name!");
    }

    size_t len_key  = strlen(MY_WIFI_AP_KEY);
    wifi_ap_key = new char[len_key+1];
    strncpy(wifi_ap_key, MY_WIFI_AP_KEY, len_key);
    if (save_ap_key()) {
      DPRINTLN("Saved wifi_ap_key");
    } else {
      DPRINTLN("Failed to save wifi_ap_key!");
    }

  }

  //
  // Connect to Wifi
  //

  connect_wifi();

  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);

}

void loop() {

  // Has its own internal check to only poll every 60s
  ntpclient.update();

  uint32_t ntp_time = ntpclient.getRawTime();
  uint32_t rtc_time = rtc.now().unixtime();

  DateTime ntp_dt = DateTime(ntp_time);
  DateTime rtc_dt = DateTime(rtc_time);

  // Check if RTC is far enough off to require adjustment
  if (rtc_time > ntp_time || ntp_time - rtc_time > 60) {
    rtc.adjust(ntp_dt);
  }

  // Fart out timestamp
  char buff[64];
  ntp_dt.toString(buff, sizeof(buff));
  DPRINT(buff);
  DPRINT(" ");
  rtc_dt.toString(buff, sizeof(buff));
  DPRINTLN(buff);

  // Update display
  displayTime(&ntp_dt);

  digitalWrite(LED_BUILTIN, led_state);
  led_state = !led_state;

  delay(1000);

}
