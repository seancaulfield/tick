/*
 * esp8266_ohai.ino - Something of a WiFi Hello World.
 *
 */

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <FS.h>
#include <Bounce2.h>

#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#include <RTClib.h>
#include <DateTime.h>
#include <DS3231.h>
#include <NTPClient.h>

#include <string.h>

#include "debug.h"
#include "wifi_setup.h"

#define DISP_ADDR              0x70
#define DISP_BRIGHT            15   //0-15
#define LED_PIN                0
#define BUTT_PIN               12
#define BUTT_DEBOUNCE_MS       200
#define PROBE_DELAY_SUCCESS_MS 5
#define PROBE_DELAY_FAIL_MS    0

//
// TODO Define these for yourself!
//
const char *MY_WIFI_AP_NAME = "<SSID>";
const char *MY_WIFI_AP_KEY  = "<KEY>";

//
// Global controls/vars
//

boolean i2c_polling = false;
NTPClient ntpclient;
Adafruit_7segment display = Adafruit_7segment();
Bounce butt = Bounce();
uint32_t last_ts = 0;
RTC_DS3231 *rtc = NULL;

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
 * i2cProbe - Twiddle the i2c bus to see if there's a RTC we recognize. Returns
 * the FIRST address from its search list that responds with an ACK. Zero if no
 * responding values found.
 *
 */
uint8_t i2cProbe() {
  uint8_t addr = DS3231_ADDRESS;
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    DPRINT(F("RTC found at "));
    DPRINTLN(addr, HEX);
    delay(PROBE_DELAY_SUCCESS_MS);
    return addr; // Match on FIRST responding address
  } else {
    DPRINT(F("RTC not found at "));
    DPRINTLN(addr, HEX);
    delay(PROBE_DELAY_FAIL_MS);
  }
  return 0;
}

/*
 * getRTC - Spin up an RTC object if it responded to a ping at the given
 * address. Somewhat problematic when needing to detect different devices with
 * the same address and different register maps (DS1307 and DS3231, OF
 * COURSE...).
 *
 * So. Mark it as a TODO to figure that out. Assuming DS3231 for now.
 *
 */
RTC_DS3231 *getRTC() {
  RTC_DS3231 *rtc = new RTC_DS3231();
  DPRINTLN(F("Enabling RTC oscillator"));
  rtc->begin();
  rtc->enable();
  return rtc;
}

/*
 * setRTC - Sync the newly probed RTC with a hot, fresh timestamp from NTP.
 */
void setRTC(RTC_DS3231 *rtc) {
  char buff[64];

  DPRINTLN(F("Forcing NTP update to set RTC"));
  ntpclient.forceUpdate();
  uint32_t ntp_raw = ntpclient.getRawTime();
  DateTime ntp_dt = DateTime(ntp_raw);

  DPRINT(F("Setting RTC to "));
  DPRINTLN(ntp_dt.toString(&buff[0], sizeof(buff)));
  rtc->adjust(ntp_dt);
  delay(100);

  DPRINTLN(F("Reading RTC value"));
  DateTime rtc_dt = rtc->now();
  DPRINT(F("RTC is "));
  DPRINTLN(rtc_dt.toString(&buff[0], sizeof(buff)));

  DPRINTLN(F("Done"));
}

/*
 * setup - Code entry point, runs once.
 */
void setup() {

#ifdef DEBUG
  setup_serial();
#endif

  // Configure button debouncer
  butt.attach(BUTT_PIN, INPUT_PULLUP, BUTT_DEBOUNCE_MS);

  // Set to output for status indication
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  // Start I2C & setup display
  Wire.begin();
  display.begin(DISP_ADDR);
  display.clear();

  // Mount config filesystem
  if (!SPIFFS.begin()) {
    DPRINTLN(F("Failed to mount config fs!"));
    return;
  }

  // If config doesn't exist, save it using compiledin values
  if (!setup_wifi()) {

    size_t len_name = strlen(MY_WIFI_AP_NAME);
    wifi_ap_name = new char[len_name+1];
    strncpy(wifi_ap_name, MY_WIFI_AP_NAME, len_name);
    if (save_ap_name()) {
      DPRINTLN(F("Saved wifi_ap_name"));
    } else {
      DPRINTLN(F("Failed to save wifi_ap_name!"));
    }

    size_t len_key  = strlen(MY_WIFI_AP_KEY);
    wifi_ap_key = new char[len_key+1];
    strncpy(wifi_ap_key, MY_WIFI_AP_KEY, len_key);
    if (save_ap_key()) {
      DPRINTLN(F("Saved wifi_ap_key"));
    } else {
      DPRINTLN(F("Failed to save wifi_ap_key!"));
    }

  }

  // Connect to Wifi
  connect_wifi();

}

void loop() {
  uint32_t ntp_raw;
  DateTime ntp_dt, rtc_dt;

  // Check for button press to start i2c polling
  butt.update();
  if (butt.fell()) {
    i2c_polling = !i2c_polling;
    DPRINTLN(F("Entering i2c polling mode"));
  }

  // If polling, see if anything responds
  if (i2c_polling) {
    uint8_t addr = i2cProbe();
    if (addr) { // Found a device!
      digitalWrite(LED_PIN, LOW); // Turn LED on
      if (rtc) delete rtc; // If RTC object already exists, delete first
      rtc = getRTC();
      setRTC(rtc);
      digitalWrite(LED_PIN, HIGH);
      i2c_polling = false;
    }
  }

  // Has its own internal check to only poll every 60s
  ntpclient.update();
  ntp_raw = ntpclient.getRawTime();
  ntp_dt = DateTime(ntp_raw);

  // Get RTC time if RTC found
  if (rtc) {
    rtc_dt = rtc->now();
  }

  // Update display if timestamps have changed
  if (ntp_raw != last_ts) {
    last_ts = ntp_raw;
    displayTime(&ntp_dt);
  }

  // Set LED on if we're currently polling to give some feedback
  //digitalWrite(LED_PIN, (i2c_polling ? HIGH : LOW));
  if (rtc) {
    DPRINT(F("NTP: ")); DPRINT(ntp_dt.iso8601());
    DPRINT(F(" RTC: ")); DPRINTLN(rtc_dt.iso8601());
  } else {
    DPRINT(F("NTP: ")); DPRINTLN(ntp_dt.iso8601());
  }
  delay(1000);

}
