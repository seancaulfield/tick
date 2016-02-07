/*
 * config_file.cpp - Configuration support routines
 *
 */

#include <FS.h>
#include "config_file.h"
#include "debug.h"

char* get_file_data(const char *filepath, size_t max_size) {

  File file = SPIFFS.open(filepath, "r");
  if (!file) {
    DPRINT("Failed to open "); DPRINT(filepath); DPRINTLN("!");
    return NULL;
  }

  size_t filesize = file.size();
  if (filesize > max_size) {
    DPRINT("Too large: "); DPRINTLN(filepath);
    return NULL;
  }

  char *buff = new char[filesize];
  file.readBytes(buff, filesize);
  if (filesize - 1 > 0) {
    buff[filesize-1] = '\0'; // ensure null termination
  }

  DPRINT("Successfully loaded file: "); DPRINTLN(filepath);
  DPRINTLN(buff);

  return buff;
}

bool put_file_data(const char *filepath, const uint8_t *data, size_t len) {

  File file = SPIFFS.open(filepath, "w");
  if (!file) {
    DPRINT("Failed to open "); DPRINT(filepath); DPRINTLN("!");
    return false;
  }

  size_t len_written = file.write(data, len);
  if (len_written != len) {
    DPRINT("put_file_data: ");
    DPRINT(filepath);
    DPRINT(": tried to write ");
    DPRINT(len);
    DPRINT(" bytes, but only wrote ");
    DPRINTLN(len_written);
    return false;
  }

  DPRINT("Successfully saved file: "); DPRINTLN(filepath);
  return true;

}
