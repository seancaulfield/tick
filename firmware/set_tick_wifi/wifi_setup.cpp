/*
 * wifi_setup.cpp - Wifi support functions, config, etc.
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <FS.h>
#include <string.h>

#include "debug.h"
#include "config_file.h"

ESP8266WiFiMulti wifim;
char *wifi_ap_name;
char *wifi_ap_key;

const char *FILENAME_WIFI_AP_NAME = "/wifi_ap_name.txt";
const char *FILENAME_WIFI_AP_KEY  = "/wifi_ap_key.txt";

char* load_ap_name() {
  wifi_ap_name = get_file_data(FILENAME_WIFI_AP_NAME);
  return wifi_ap_name;
}

char* load_ap_key() {
  wifi_ap_key = get_file_data(FILENAME_WIFI_AP_KEY);
  return wifi_ap_key;
}

bool save_ap_name() {
  return put_file_data(FILENAME_WIFI_AP_NAME, (uint8_t*)wifi_ap_name, strlen(wifi_ap_name));
}

bool save_ap_key() {
  return put_file_data(FILENAME_WIFI_AP_KEY, (uint8_t*)wifi_ap_key, strlen(wifi_ap_key));
}

bool setup_wifi() {
  if (load_ap_name() == NULL || load_ap_key() == NULL) {
    DPRINTLN("Unable to load wifi config, skipping!");
    return false;
  }
  return true;
}

bool connect_wifi() {

  wifim.addAP(wifi_ap_name, wifi_ap_key);

  DPRINT("Connecting to \""); DPRINT(wifi_ap_name); DPRINT("\" ...");
  DFLUSH();

  while (wifim.run() != WL_CONNECTED) {
    DPRINT(".");
    DFLUSH();
    delay(500);
  }

  DPRINTLN("Connected!");
  DPRINT("IP: ");
  DPRINTLN(WiFi.localIP());

  return true;

}
