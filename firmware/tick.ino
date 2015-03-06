#include <Arduino.h>
#include <TinyWireM.h>
#define Wire TinyWireM
#include <RTCLib/DS3231.h>
#include <Time.h>
#include <Timezone.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>

#define DO_1_TIME_SETUP 0
#define SERIAL_TIME_SET 0

// ------------------------------------------------------------
// GLOBAL CONSTANTS/VARIABLES
// ------------------------------------------------------------

// Display i2c addressing
//
// Base = 0x70
// A0 = 0x01
// A1 = 0x02
// A2 = 0x04
//

const int DISPLAY0_ADDR = 0x70;
const int DISPLAY1_ADDR = 0x71; // A0
const int DISPLAY2_ADDR = 0x72; // A1
const int DISPLAY_BRIGHTNESS = 15; //0-15
const int DISPLAY_COLON_LOC = 2;
const int DISPLAY_COLON_BITS = 0x02;

const int CLOCK_ADDR = 0x68;
const int BUFF_SIZE = 16;

RTC_DS3231 rtc = RTC_DS3231();
Adafruit_7segment display0 = Adafruit_7segment();
Adafruit_7segment display1 = Adafruit_7segment();
Adafruit_7segment display2 = Adafruit_7segment();

TimeChangeRule est = {"EST", First, Sun, Nov, 2, -5*60};
TimeChangeRule edt = {"EDT", Second, Sun, Mar, 2, -4*60};
Timezone tz = Timezone(edt, est);

// ------------------------------------------------------------
// "HELPING" FUNCTIONS
//
// http://theprofoundprogrammer.com/post/31046915750/text-im-pretty-sure-that-im-helping
// ------------------------------------------------------------

void displayYear(DateTime *dt) {
  display2.clear();
  display2.writeDigitNum(0, (dt->year() / 1000) % 10, false);
  display2.writeDigitNum(1, (dt->year() / 100) % 10, false);
  display2.writeDigitNum(3, (dt->year() / 10) % 10, false);
  display2.writeDigitNum(4, dt->year() % 10, true); //decimal to split year/month
  display2.writeDisplay();
}

void displayDate(DateTime *dt) {
  display1.clear();
  if (dt->month() >= 10)
    display1.writeDigitNum(0, (dt->month() / 10) % 10, false);
  display1.writeDigitNum(1, dt->month() % 10, true); //decimal to split month/day
  if (dt->day() >= 10)
    display1.writeDigitNum(3, (dt->day() / 10) % 10, false);
  display1.writeDigitNum(4, dt->day() % 10, false);
  display1.writeDisplay();
}

void displayTime(DateTime *dt) {
  display0.clear();
  if (dt->hour() >= 10)
    display0.writeDigitNum(0, (dt->hour() / 10) % 10, false);
  display0.writeDigitNum(1, dt->hour() % 10, false);
  display0.writeDigitRaw(DISPLAY_COLON_LOC, DISPLAY_COLON_BITS);
  display0.writeDigitNum(3, dt->minute() / 10, false);
  display0.writeDigitNum(4, dt->minute() % 10, false);
  display0.writeDisplay();
}

#if DO_1_TIME_SETUP
void oneTimeSetup() {

  //
  // Write to control register to clear extra crap to save power.
  //
  // a) enable oscillator (/EOSC), which is set to 0 to enable it
  // b) battery backed square wave is off (BBSQW, bit 6)
  // c) don't ask for temp correct to run right now (it will keep going in the
  // background which is good enough)
  // d) set square wave output to 1Hz (RS1 & RS2, bits 3-4)
  // e) disable interrupt (INTCN, bit 2)
  // f) disable alarms (A1IE & A2IE, bits 0-1)
  //
  // ...which all boils down to setting the damn thing to 0. Woo!
  //
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_REG_CONTROL);
  Wire.write(0x00);
  Wire.endTransmission();

  //
  // Clear out last other control thingy, the 32kHz output (EN32K, bit 3)
  //
  Wire.beginTransmission(DS3231_ADDRESS);
  Wire.write(DS3231_REG_STATUS_CTL);
  Wire.write(0x00);
  Wire.endTransmission();

}
#endif

#if SERIAL_TIME_SET
void serialSetTime() {
  time_t t = 0;

  if (Serial.available() && Serial.find("T")) {
    t = Serial.parseInt();
  }

  if (t != 0) {
    rtc.adjust(DateTime(t));
  }
}
#endif

// ------------------------------------------------------------
// SETUP() - THIS THING IS DONE ONCE
// ------------------------------------------------------------

void setup() {

  // Setup serial connection for debugging/time setting
#if SERIAL_TIME_SET
  Serial.begin(9600);
#endif

  Wire.begin();
  rtc.begin();

  display0.begin(DISPLAY0_ADDR);
  display0.setBrightness(DISPLAY_BRIGHTNESS);
  display0.clear();

  display1.begin(DISPLAY1_ADDR);
  display1.setBrightness(DISPLAY_BRIGHTNESS);
  display1.clear();

  display2.begin(DISPLAY2_ADDR);
  display2.setBrightness(DISPLAY_BRIGHTNESS);
  display2.clear();

}

// ------------------------------------------------------------
// LOOP() - THIS THING IS DONE LOTS, LIKE YOUR MOM
// ------------------------------------------------------------

void loop() {

#if DO_1_TIME_SETUP
  oneTimeSetup()
#endif

#if SERIAL_TIME_SET
  serialSetTime();
#endif

  uint32_t utc = rtc.now().unixtime();
  DateTime now = DateTime(tz.toLocal(utc));
  displayTime(&now);
  displayDate(&now);
  displayYear(&now);

  delay(100);
}

// vi: syntax=arduino
