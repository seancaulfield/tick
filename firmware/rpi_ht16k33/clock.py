#!/usr/bin/env python

import sys
import time
import datetime
import daemon
import random
import logging
import logging.handlers
import signal
import traceback

from Adafruit_LED_Backpack.SevenSegment import SevenSegment

DP_UPPER   = 0
DP_LOWER_L = 1
DP_LOWER_R = 2

DISP_ADDRS = {
    DP_UPPER   : 0x70,
    DP_LOWER_L : 0x71,
    DP_LOWER_R : 0x72,
}

LOG_FORMAT  = "%(filename)s[%(process)d]: %(message)s"
LOG_FORMAT += " (%(funcName)s:%(lineno)d)"

SYSLOG_SOCK = "/dev/log"

terminated = False
woke = False

def blank(displays):

    displays[DP_LOWER_R].clear()
    displays[DP_LOWER_R].write_display()

    displays[DP_LOWER_L].clear()
    displays[DP_LOWER_L].write_display()

    displays[DP_UPPER].clear()
    displays[DP_UPPER].write_display()

def refresh(displays, dateSep=True, timeSep=True):

    now = datetime.datetime.now()

    displays[DP_LOWER_R].clear()
    displays[DP_LOWER_R].set_digit(0, now.month / 10)
    displays[DP_LOWER_R].set_digit(1, now.month % 10, decimal=dateSep)
    displays[DP_LOWER_R].set_digit(2, now.day / 10)
    displays[DP_LOWER_R].set_digit(3, now.day % 10, decimal=dateSep)
    displays[DP_LOWER_R].write_display()

    displays[DP_LOWER_L].clear()
    displays[DP_LOWER_L].set_digit(0, (now.year / 1000) % 10)
    displays[DP_LOWER_L].set_digit(1, (now.year / 100)  % 10)
    displays[DP_LOWER_L].set_digit(2, (now.year / 10)   % 10)
    displays[DP_LOWER_L].set_digit(3, (now.year)        % 10, decimal=dateSep)
    displays[DP_LOWER_L].write_display()

    displays[DP_UPPER].clear()
    displays[DP_UPPER].set_digit(0, now.hour / 10)
    displays[DP_UPPER].set_digit(1, now.hour % 10)
    displays[DP_UPPER].set_digit(2, now.minute / 10)
    displays[DP_UPPER].set_digit(3, now.minute % 10)
    displays[DP_UPPER].set_colon(timeSep)
    displays[DP_UPPER].write_display()

def wake_handler(signum, frame):
    global woke
    woke = True

def exit_handler(signum, frame):
    global terminated
    terminated = True

def between(x, y, z):
    """Is X >= Y AND X < Z?"""
    return (x >= y and x < z)

def log_tb(logger, e):
    logger.fatal(e)
    for line in traceback.format_exc().split("\n"):
        logger.fatal(line)

def marquee(displays, text, left=True):
    end = len(text)
    char_if_in_bounds = lambda x: text[x] if (x >= 0 and x < end) else ' '
    displays[DP_UPPER].clear()
    for i in range(0, end+4):
        if left:
            displays[DP_UPPER].set_digit(3, char_if_in_bounds(i))
            displays[DP_UPPER].set_digit(2, char_if_in_bounds(i - 1))
            displays[DP_UPPER].set_digit(1, char_if_in_bounds(i - 2))
            displays[DP_UPPER].set_digit(0, char_if_in_bounds(i - 3))
        else:
            displays[DP_UPPER].set_digit(0, char_if_in_bounds(end - i))
            displays[DP_UPPER].set_digit(1, char_if_in_bounds(end - i + 1))
            displays[DP_UPPER].set_digit(2, char_if_in_bounds(end - i + 2))
            displays[DP_UPPER].set_digit(3, char_if_in_bounds(end - i + 3))
        displays[DP_UPPER].write_display()
        time.sleep(0.3)

def main(logger):
    global terminated
    global woke

    logger.info("clock.py starting")

    displays = [SevenSegment(address=a) for n, a in DISP_ADDRS.items()]
    for d in displays:
        d.begin()
        d.clear()
        d.write_display()

    terminated = False
    woke = False
    while not terminated:
        refresh(displays)
        seconds = 60 - datetime.datetime.now().second
        woke = False
        signal.alarm(seconds)
        signal.pause()
        if woke:
            logger.debug("Woken up")

    blank(displays)
    logger.info("clock.py stopping")

if __name__ == '__main__':

    with daemon.DaemonContext() as demon:

        # Setup logging to syslog
        logger = logging.getLogger()
        logfmt = logging.Formatter(LOG_FORMAT)
        syslog = logging.handlers.SysLogHandler(SYSLOG_SOCK)
        syslog.setFormatter(logfmt)
        syslog.setLevel(logging.INFO)
        logger.setLevel(logging.INFO)
        logger.addHandler(syslog)

        # Setup signal handlers for termination
        signal.signal(signal.SIGTERM, exit_handler)
        signal.signal(signal.SIGINT, exit_handler)
        signal.signal(signal.SIGHUP, wake_handler)
        signal.signal(signal.SIGALRM, wake_handler)

        # Main retry loop, which will retry on IOErrors (since that's what the
        # Adafruit library seems to throw when one of the displays doesn't
        # respond over i2c) and exit on others.
        while True:
            try:
                main(logger)
            except IOError, ioe:
                log_tb(logger, e)
                continue
            except Exception, e:
                log_tb(logger, e)
                break
