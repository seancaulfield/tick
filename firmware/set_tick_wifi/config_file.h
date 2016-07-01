/*
 * config_file.h - Configuration support routines
 *
 */

#ifndef __CONFIG_FILE_H
#define __CONFIG_FILE_H

#include <FS.h>
#include "debug.h"

char* get_file_data(const char *filepath, size_t max_size=1024);
bool  put_file_data(const char *filepath, const uint8_t *data, size_t len);

#endif
