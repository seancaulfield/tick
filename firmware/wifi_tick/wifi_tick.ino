/*
 * esp8266_ohai.ino - Something of a WiFi Hello World.
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <FS.h>
#include <LedControl.h>
#include <string.h>

#include "debug.h"
#include "wifi_setup.h"

// Pin assignments

#define DISP_DATA 13
#define DISP_CLCK 14
#define DISP_LOAD 16
#define DISP_NUM  1

//
// TODO Define these for yourself!
//
const char *MY_WIFI_AP_NAME = "<SSID>";
const char *MY_WIFI_AP_KEY  = "<KEY>";

//
// Global controls/vars
//

LedControl display = LedControl(DISP_DATA, DISP_CLCK, DISP_LOAD, DISP_NUM);

void setup_serial() {
  delay(200);
  Serial.begin(SERIAL_BAUD);
  delay(10);
}

void setup() {

#ifdef DEBUG
  setup_serial();
#endif

  //
  // Config probably doesn't exist, so try to save it to SPIFFS
  //

  if (!setup_wifi()) {

    size_t len_name = strlen(MY_WIFI_AP_NAME) + 1;
    wifi_ap_name = new char[len_name];
    strncpy(wifi_ap_name, MY_WIFI_AP_NAME, len_name);
    if (save_ap_name()) {
      DPRINTLN("Saved wifi_ap_name");
    } else {
      DPRINTLN("Failed to save wifi_ap_name!");
    }

    size_t len_key  = strlen(MY_WIFI_AP_KEY) + 1;
    wifi_ap_key = new char[len_key];
    strncpy(wifi_ap_key, MY_WIFI_AP_KEY, len_key);
    if (save_ap_key()) {
      DPRINTLN("Saved wifi_ap_key");
    } else {
      DPRINTLN("Failed to save wifi_ap_key!");
    }

  }

  delay(500);

}

#if 0
void tcp_echo() {
  const uint16_t port = 8081;
  const char *hostname = "athena.lan";

  DPRINT("Connecting to ");
  DPRINTLN(hostname);

  WiFiClient client;
  if (!client.connect(hostname, port)) {
    DPRINTLN("Connection failed!");
    return;

  }

  // GREETINGS EARTHLING
  DPRINT("> HAI");
  client.println("HAI");

  bool failure = false;
  uint32_t timeout = 30 * 1000 + millis(); // 30 second timeout
  while (!client.available()) {
    if (timeout - millis() < 0) {
      DPRINTLN("Timeout waiting for client");
      failure = true;
    }
  }

  if (!failure) {
    while (client.available()) {
      String stuff = client.readStringUntil('\n');
      client.println(stuff);
      DPRINT("> ");
      DPRINTLN(stuff);
    }
  }

  DPRINTLN("Closing...");
  client.stop();

}
#endif

void loop() {
  //tcp_echo();
  delay(5000);
}

