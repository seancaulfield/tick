#!/usr/bin/python3

import sys
import time
import datetime
import logging
import logging.handlers
import signal
import traceback

import board
from adafruit_ht16k33.segments import (
    Seg7x4,
    BigSeg7x4,
    Seg14x4,
)


# Do we have an alphanumeric 2-digit display for the day of week?
HAS_DOW = True

# Do we log stack traces upon exceptions to syslog?
LOG_STACK_TRACE = True


# Class to slightly change behavior of alphanumeric class to handle two digit
# displays
class Seg14x2(Seg14x4):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, chars_per_display=2, **kwargs)


# Semantic mapping of displays
DP_LOWER_L = 0
DP_LOWER_R = 1
DP_UPPER   = 2
DP_ALNUM   = 3

# Mapping semantic names to classes and I2C addresses
DISP_CONF = {
    DP_LOWER_L : (Seg7x4, 0x70),
    DP_LOWER_R : (Seg7x4, 0x71),
    DP_UPPER   : (BigSeg7x4, 0x72),
    DP_ALNUM   : (Seg14x2, 0x73),
}

DAYS_OF_WEEK = [
    "Mo",
    "Tu",
    "We",
    "Th",
    "Fr",
    "Sa",
    "Su",
]

LOG_FORMAT  = "%(filename)s[%(process)d]: %(message)s"
LOG_FORMAT += " (%(funcName)s:%(lineno)d)"

SYSLOG_SOCK = "/dev/log"

terminated = False
blinked = False


def blink(displays):
    for d in displays.values():
        d.fill(True)

def blank(displays):
    for d in displays.values():
        d.fill(False)
        d.show()

def refresh(now, displays, dateSep=True, timeSep=True):

    displays[DP_LOWER_R][0] = str(now.month // 10)
    displays[DP_LOWER_R][1] = str(now.month % 10)
    displays[DP_LOWER_R][2] = str(now.day // 10)
    displays[DP_LOWER_R][3] = str(now.day % 10)
    if dateSep:
        displays[DP_LOWER_R]._put('.', 1)
    displays[DP_LOWER_R].show()

    displays[DP_LOWER_L][0] = str((now.year // 1000) % 10)
    displays[DP_LOWER_L][1] = str((now.year // 100)  % 10)
    displays[DP_LOWER_L][2] = str((now.year // 10)   % 10)
    displays[DP_LOWER_L][3] = str(now.year % 10)
    if dateSep:
        displays[DP_LOWER_L]._put('.', 3)
    displays[DP_LOWER_L].show()

    displays[DP_UPPER][0] = str(now.hour // 10)
    displays[DP_UPPER][1] = str(now.hour % 10)
    displays[DP_UPPER][2] = str(now.minute // 10)
    displays[DP_UPPER][3] = str(now.minute % 10)
    displays[DP_UPPER].colons[0] = timeSep
    displays[DP_UPPER].show()

    if HAS_DOW:
        displays[DP_ALNUM].print(DAYS_OF_WEEK[now.weekday()])
        displays[DP_ALNUM].show()

def make_display(i2c, cls, addr, auto_write=False):
    return cls(i2c, address=addr, auto_write=auto_write)

def blink_handler(signum, frame):
    global blinked
    blinked = True

def wake_handler(signum, frame):
    pass

def exit_handler(signum, frame):
    global terminated
    terminated = True

def main(logger):
    global terminated
    global blinked

    logger.info("clock.py starting")

    # Setup signal handlers for termination
    signal.signal(signal.SIGTERM, exit_handler)
    signal.signal(signal.SIGINT, exit_handler)
    signal.signal(signal.SIGHUP, wake_handler)
    signal.signal(signal.SIGALRM, wake_handler)
    signal.signal(signal.SIGUSR1, blink_handler)

    # Create SevenSegment display instances
    i2c = board.I2C()
    displays = {k: make_display(i2c, *v) for k, v in DISP_CONF.items()}

    blank(displays)

    while not terminated:

        # Blink twice
        if blinked:

            # Disable the blinking for next time
            blinked = False

            blink(displays)
            time.sleep(1)
            blank(displays)
            time.sleep(0.5)

            blink(displays)
            time.sleep(1)
            blank(displays)
            time.sleep(0.5)

        # Refresh displays with current time
        now = datetime.datetime.now()
        refresh(now, displays)

        # Set wakeup for update ~one minute from now (next time the display
        # would need updated)
        signal.alarm(60 - datetime.datetime.now().second)
        signal.pause()

    blank(displays)

    logger.info("clock.py stopping")

if __name__ == '__main__':

    # Setup logging to syslog
    logger = logging.getLogger()
    logfmt = logging.Formatter(LOG_FORMAT)
    syslog = logging.handlers.SysLogHandler(SYSLOG_SOCK)
    syslog.setFormatter(logfmt)
    syslog.setLevel(logging.INFO)
    logger.setLevel(logging.INFO)
    logger.addHandler(syslog)

    # Catch errors from now on and dump them to syslog
    try:
        main(logger)
    except Exception as e:
        logger.fatal(e)
        if LOG_STACK_TRACE:
            for line in traceback.format_exc().split("\n"):
                logger.fatal(line)
