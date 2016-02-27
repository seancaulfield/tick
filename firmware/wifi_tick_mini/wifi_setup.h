/*
 * wifi_setup.h - Wifi support functions, config, etc.
 *
 */

#ifndef __WIFI_SETUP_H
#define __WIFI_SETUP_H

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <FS.h>

#include "debug.h"

extern ESP8266WiFiMulti wifim;
extern char *wifi_ap_name;
extern char *wifi_ap_key;

char* load_ap_name();
char* load_ap_key();
bool save_ap_name();
bool save_ap_key();
bool setup_wifi();
bool connect_wifi();

#endif
