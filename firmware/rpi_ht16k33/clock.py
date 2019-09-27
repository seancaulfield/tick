#!/usr/bin/env python

import sys
import time
import datetime
import random
import logging
import logging.handlers
import signal
import traceback

from Adafruit_LED_Backpack.SevenSegment import SevenSegment
from Adafruit_LED_Backpack.AlphaNum4 import AlphaNum4

DP_UPPER   = 0
DP_LOWER_L = 1
DP_LOWER_R = 2
DP_ALNUM   = 3

DISP_ADDRS = {
    DP_UPPER   : 0x70,
    DP_LOWER_L : 0x71,
    DP_LOWER_R : 0x72,
}

DISP_ADDR_ALNUM = 0x74

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

DAEMONIZE = False
HAS_DOW = False

terminated = False
blinked = False

def blink(displays):
    for d in displays:
        d.clear()
        if d._device._address == DISP_ADDR_ALNUM:
            d.set_digit(0, "*")
            d.set_digit(1, "*")
        else:
            d.set_digit(0, 8)
            d.set_digit(1, 8)
            d.set_digit(2, 8)
            d.set_digit(3, 8)
        d.write_display()

def blank(displays, begin=False):
    for d in displays:
        if begin:
            d.begin()
        d.clear()
        d.write_display()

def refresh(now, displays, dateSep=True, timeSep=True):

    displays[DP_LOWER_R].set_digit(0, now.month / 10)
    displays[DP_LOWER_R].set_digit(1, now.month % 10, decimal=dateSep)
    displays[DP_LOWER_R].set_digit(2, now.day / 10)
    displays[DP_LOWER_R].set_digit(3, now.day % 10)
    displays[DP_LOWER_R].write_display()

    displays[DP_LOWER_L].set_digit(0, (now.year / 1000) % 10)
    displays[DP_LOWER_L].set_digit(1, (now.year / 100)  % 10)
    displays[DP_LOWER_L].set_digit(2, (now.year / 10)   % 10)
    displays[DP_LOWER_L].set_digit(3, (now.year)        % 10, decimal=dateSep)
    displays[DP_LOWER_L].write_display()

    displays[DP_UPPER].set_digit(0, now.hour / 10)
    displays[DP_UPPER].set_digit(1, now.hour % 10)
    displays[DP_UPPER].set_digit(2, now.minute / 10)
    displays[DP_UPPER].set_digit(3, now.minute % 10)
    displays[DP_UPPER].set_colon(timeSep)
    displays[DP_UPPER].write_display()

    if HAS_DOW:
        displays[DP_ALNUM].set_digit(0, DAYS_OF_WEEK[now.weekday()][0])
        displays[DP_ALNUM].set_digit(1, DAYS_OF_WEEK[now.weekday()][1])
        displays[DP_ALNUM].write_display()

def blink_handler(signum, frame):
    global blinked
    blinked = True

def wake_handler(signum, frame):
    pass

def exit_handler(signum, frame):
    global terminated
    terminated = True

def log_tb(logger, e):
    logger.fatal(e)
    for line in traceback.format_exc().split("\n"):
        logger.fatal(line)

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
    displays = [SevenSegment(address=a) for n, a in DISP_ADDRS.items()]

    # Create AlphaNum4 (cheating~) display instance
    if HAS_DOW:
        displays.append(AlphaNum4(address=DISP_ADDR_ALNUM))

    blank(displays, begin=True)

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
        if DAEMONIZE:
            import daemon
            with daemon.DaemonContext() as demon:
                main(logger)
        else:
            main(logger)
    except Exception, e:
        log_tb(logger, e)
