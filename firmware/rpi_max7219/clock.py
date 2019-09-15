#!/usr/bin/env python

import sys
import time
import datetime
import dateutils
import daemon
import random
import logging
import logging.handlers
import signal
import traceback

import max7219.led
from max7219.led import constants as consts

DP_UPPER = 1
DP_LOWER = 0

LOG_FORMAT  = "%(filename)s[%(process)d]: %(message)s"
LOG_FORMAT += " (%(funcName)s:%(lineno)d)"

SYSLOG_SOCK = "/dev/log"

terminated = False

class SevenSegment(max7219.led.sevensegment):

    def __init__(self, *args, **kwargs):
        super(SevenSegment, self).__init__(*args, **kwargs)

    def command1(self, deviceId, register, data):
        """Same thing as command but allows picking a device."""
        
        buff = (consts.MAX7219_REG_NOOP, 0x00)
        for i in range(0, deviceId):
            self._write(buff)
        self._write([register, data])

    def setScanLimit(self, deviceId, limit=7):
        """Set scan limit for a particular device so display brightness matches
        if you have less than the maximum number of digits."""

        assert(0 <= limit <= 7)
        self.command1(deviceId, consts.MAX7219_REG_SCANLIMIT, limit)

    def rotated(self, deviceId, position, char, dot=False, redraw=True):
        """Same thing as letter but rotated 180 degrees."""

        assert dot in [0, 1, False, True]
        v = self._DIGITS.get(str(char), self._UNDEFINED)

        ############## BIT MAGIC IS HERE ############## 
        # 
        #     3              6   
        #   +---+          +---+
        # 2 |   | 4      5 |   | 1
        #   +-0-+    =>    +-0-+  
        # 1 |   | 5      4 |   | 2
        #   +---+          +---+
        #     6              3    
        #
        w = (0b1 & v) | ((0b1110000 & v) >> 3) | ((0b1110 & v) << 3)
        ############## END BIT MAGIC ############## 

        v = w | (dot << 7)
        self.set_byte(deviceId, position, v, redraw)

def display(dev, dateSep=True, timeSep=True):

    now = datetime.datetime.now()

    dev.letter(DP_LOWER, 3, now.day / 10, redraw=False)
    dev.letter(DP_LOWER, 4, now.day % 10, redraw=False, dot=dateSep)

    dev.letter(DP_LOWER, 1, now.month / 10, redraw=False)
    dev.letter(DP_LOWER, 2, now.month % 10, redraw=False, dot=dateSep)

    dev.letter(DP_LOWER, 5, (now.year / 1000) % 10, redraw=False)
    dev.letter(DP_LOWER, 6, (now.year / 100)  % 10, redraw=False)
    dev.letter(DP_LOWER, 7, (now.year / 10)   % 10, redraw=False)
    dev.letter(DP_LOWER, 8, (now.year)        % 10, redraw=False, dot=dateSep)

    dev.letter(DP_UPPER, 1, now.hour / 10, redraw=False)
    dev.letter(DP_UPPER, 2, now.hour % 10, redraw=False, dot=timeSep)

    dev.rotated(DP_UPPER, 3, now.minute / 10, redraw=False, dot=timeSep)
    dev.rotated(DP_UPPER, 4, now.minute % 10, redraw=False)

    dev.flush()

def wake_handler(signum, frame):
    global woke
    woke = True

def exit_handler(signum, frame):
    global terminated
    terminated = True

def between(x, y, z):
    """Is X >= Y AND X < Z?"""
    return (x >= y and x < z)

def marquee(dev, text, left=True):
    end = len(text)
    char_if_in_bounds = lambda x: text[x] if (x >= 0 and x < end) else ' '
    for i in range(0, end+4):
        if left:
            dev.rotated(DP_UPPER, 4, char_if_in_bounds(i),     redraw=False)
            dev.rotated(DP_UPPER, 3, char_if_in_bounds(i - 1), redraw=False)
            dev.letter(DP_UPPER,  2, char_if_in_bounds(i - 2), redraw=False)
            dev.letter(DP_UPPER,  1, char_if_in_bounds(i - 3), redraw=False)
        else:
            dev.letter(DP_UPPER,  1, char_if_in_bounds(end - i),     redraw=False)
            dev.letter(DP_UPPER,  2, char_if_in_bounds(end - i + 1), redraw=False)
            dev.rotated(DP_UPPER, 3, char_if_in_bounds(end - i + 2), redraw=False)
            dev.rotated(DP_UPPER, 4, char_if_in_bounds(end - i + 3), redraw=False)
        dev.flush()
        time.sleep(0.3)

def hello(dev):
    dev.clear()
    marquee(dev, 'hELLO', False)
    dev.clear()

def goodbye(dev):
    dev.clear()
    marquee(dev, 'gOOdbyE')
    dev.clear()

def main(logger):
    global terminated
    global woke

    signal.signal(signal.SIGTERM, exit_handler)
    signal.signal(signal.SIGINT, exit_handler)
    signal.signal(signal.SIGHUP, wake_handler)
    signal.signal(signal.SIGALRM, wake_handler)

    dev = SevenSegment(cascaded=2)
    dev.setScanLimit(DP_UPPER, 4)

    logger.info("Hello, clock.py starting")
    hello(dev)

    terminated = False
    woke = False
    while not terminated:
        display(dev)
        seconds = 60 - datetime.datetime.now().second
        woke = False
        signal.alarm(seconds)
        signal.pause()
        if woke:
            logger.debug("Woken up")

    logger.info("clock.py stopping, Goodbye")
    goodbye(dev)

if __name__ == '__main__':

    with daemon.DaemonContext() as demon:

        logger = logging.getLogger()
        logfmt = logging.Formatter(LOG_FORMAT)
        syslog = logging.handlers.SysLogHandler(SYSLOG_SOCK)
        syslog.setFormatter(logfmt)
        syslog.setLevel(logging.INFO)
        logger.setLevel(logging.INFO)
        logger.addHandler(syslog)

        try:
            main(logger)
        except Exception, e:
            logger.fatal(e)
            for line in traceback.format_exc().split("\n"):
                logger.fatal(line)

