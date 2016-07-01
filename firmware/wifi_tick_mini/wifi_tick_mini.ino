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
#include <Time.h>
#include <Timezone.h>
#include <NTPClient.h>

#include <string.h>

#include "debug.h"
#include "wifi_setup.h"

#define DISP_ADDR   0x70
#define DISP_BRIGHT 15   //0-15

//
// TODO Define these for yourself!
//
const char *MY_WIFI_AP_NAME = "<SSID>";
const char *MY_WIFI_AP_KEY  = "<KEY>";

//
// Global controls/vars
//

boolean led_state = false;
NTPClient ntpclient;
Adafruit_7segment display = Adafruit_7segment();

/*
 * setup_serial - Initialize serial connection for debugging (if enabled).
 */
void setup_serial() {
  delay(200);
  Serial.begin(SERIAL_BAUD);
  delay(10);
}

/*
 * displayTime - Output time to segmented display controller.
 *
 * @param dt DateTime to display
 *
 */
void displayTime(DateTime *dt) {
  byte hour = (dt->hour()) % 24;
  display.clear();
  if (hour >= 20) {
    display.writeDigitNum(0, 2, false);
  } else if (hour >= 10) {
    display.writeDigitNum(0, 1, false);
  }
  display.writeDigitNum(1, hour % 10, false);
  display.writeDigitRaw(2, 0x02); //colon
  display.writeDigitNum(3, dt->minute() / 10, false);
  display.writeDigitNum(4, dt->minute() % 10, false);
  display.writeDisplay();
}

/* 
 * test_display - Cycle through segments to make sure display is working.
 */
void test_display() {
  for (byte j=0; j < 5; j++) {
    for (byte k=0; k < 8; k++) {
      byte m = 1 << k;
      display.clear();
      display.writeDigitRaw(j, m);
      display.writeDisplay();
      delay(100);
    }
  }
}

/*
 * setup - Code entry point, runs once.
 */
void setup() {

#ifdef DEBUG
  setup_serial();
#endif

  //
  // Start I2C & setup display
  //
  Wire.begin();
  display.begin(DISP_ADDR);
  display.clear();

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

#if DEBUG
  delay(500);
  pinMode(LED_BUILTIN, OUTPUT);
#endif

}

void loop() {

  //test_display();

  // Has its own internal check to only poll every 60s
  ntpclient.update();

  uint32_t ntp_time = ntpclient.getRawTime();
  //uint32_t rtc_time = rtc.now().unixtime();

  DateTime ntp_dt = DateTime(ntp_time);
  //DateTime rtc_dt = DateTime(rtc_time);

  // Check if RTC is far enough off to require adjustment
  //if (rtc_time > ntp_time || ntp_time - rtc_time > 60) {
  //  rtc.adjust(ntp_dt);
  //}

  // Fart out timestamp
  char buff[64];
  ntp_dt.toString(buff, sizeof(buff));
  //DPRINT(buff);
  //DPRINT(" ");
  //rtc_dt.toString(buff, sizeof(buff));
  DPRINTLN(buff);

  // Update display
  displayTime(&ntp_dt);

  digitalWrite(LED_BUILTIN, led_state);
  led_state = !led_state;

  delay(1000);

}
