USER_LIB_PATH := $(realpath ./lib)
ARDUINO_LIBS  := RTClib RTClib/DS3231 Time Timezone Wire
BOARD_TAG     := uno
ARDUINO_PORT  := /dev/ttyUSB0
ISP_PORT      := /dev/ttyUSB0
CXXFLAGS      += -std=gnu++11
CFLAGS        += -std=gnu++11
AVRDUDE_ISP_BAUDRATE := 19200

include /usr/share/arduino/Arduino.mk
