#!/usr/bin/python
# ======================================================================
# template.py - USBtiny/template test program
#
# Copyright 2006-2010 Dick Streefland
# ======================================================================

import sys, os.path, datetime, math
sys.path[0] = os.path.join(sys.path[0], '../util')
import usbtiny

def dow_get_time(dev):
	"""Gets the current time from the Day-of-Week Clock, returning it as a datetime."""
	r = dev.control_in(1, 0x0, 0, 8)
	year = 2000 + ((ord(r[7]) >> 4) * 10) + (ord(r[7]) & 0x0F) + (((ord(r[3]) >> 6) & 0x1) * 100)
	month = ((ord(r[6]) >> 4) * 10) + (ord(r[6]) & 0x0F)
	day = ((ord(r[5]) >> 4) * 10) + (ord(r[5]) & 0x0F)
	hour = (((ord(r[3]) & 0x30) >> 4) * 10) + (ord(r[3]) & 0x0F)
	minute = ((ord(r[2]) >> 4) * 10) + (ord(r[2]) & 0x0F)
	second = ((ord(r[1]) >> 4) * 10) + (ord(r[1]) & 0x0F)
	microsecond = ((ord(r[0]) >> 4) * 100000) + ((ord(r[0]) & 0x0F) * 10000)
	return datetime.datetime(year, month, day, hour, minute, second, microsecond)

def dow_set_time(dev, new_time):
	"""Sets the current time of the Day-of-Week Clock to new_time."""
	year = max(new_time.year - 2000, 0);
	
	raw_time = chr(((new_time.microsecond / 100000) << 4) + ((new_time.microsecond / 10000) % 10))
	raw_time = raw_time + chr(((new_time.second / 10) << 4) + (new_time.second % 10))
	raw_time = raw_time + chr(((new_time.minute / 10) << 4) + (new_time.minute % 10))
	raw_time = raw_time + chr(((new_time.hour / 10) << 4) + (new_time.hour % 10) + 0x80 + ((year / 100) * 0x40))
	raw_time = raw_time + chr(new_time.isoweekday())
	raw_time = raw_time + chr(((new_time.day / 10) << 4) + (new_time.day % 10))
	raw_time = raw_time + chr(((new_time.month / 10) << 4) + (new_time.month % 10))
	raw_time = raw_time + chr((((year % 100) / 10) << 4) + year % 10)
	usbtiny.dump(0, raw_time)
	dev.control_out(2, 0, 0, raw_time)

def dow_set_alarm(dev, new_time, match_criteria):
	"""Sets the alarm on the Day-of-Week Clock to new_time and the repeat bits to the value of repeat."""
	regs = dev.control_in(3, 0xA, 0, 6)
	regs = list(regs)
	if new_time is None:
		regs[0] = chr(((ord(regs[0]) | 0x80) & 0xE0))
		regs[1] = chr(((match_criteria & 0x10) << 2) + ((match_criteria & 0x08) << 4))
		regs[2] = chr(((match_criteria & 0x04) << 5) + (ord(regs[2]) & 0x40))
		regs[3] = chr(((match_criteria & 0x02) << 6))
		regs[4] = chr(((match_criteria & 0x01) << 7))
	elif isinstance(new_time, datetime.datetime):
		regs[0] = chr(((ord(regs[0]) | 0x80) & 0xE0) | (((new_time.month / 10) << 4) + (new_time.month % 10)));
		regs[1] = chr(((new_time.day / 10) << 4) + (new_time.day % 10) + ((match_criteria & 0x10) << 2) + ((match_criteria & 0x08) << 4))
		regs[2] = chr(((match_criteria & 0x04) << 5) + (ord(regs[2]) & 0x40) + ((new_time.hour / 10) << 4) + (new_time.hour % 10))
		regs[3] = chr(((new_time.minute / 10) << 4) + (new_time.minute % 10) + ((match_criteria & 0x02) << 6))
		regs[4] = chr(((new_time.second / 10) << 4) + (new_time.second % 10) + ((match_criteria & 0x01) << 7))
	else:
		regs[0] = chr(((ord(regs[0]) | 0x80) & 0xE0))
		regs[1] = chr(((match_criteria & 0x10) << 2) + ((match_criteria & 0x08) << 4))
		regs[2] = chr(((match_criteria & 0x04) << 5) + (ord(regs[2]) & 0x40) + ((new_time.hour / 10) << 4) + (new_time.hour % 10))
		regs[3] = chr(((new_time.minute / 10) << 4) + (new_time.minute % 10) + ((match_criteria & 0x02) << 6))
		regs[4] = chr(((new_time.second / 10) << 4) + (new_time.second % 10) + ((match_criteria & 0x01) << 7))
	regs = "".join(regs)
	dev.control_out(4, 0xA, 0, regs[0:5])
	dev.control_in(3, 0xA, 0, 1) # move address pointer away from 0xF

def dow_alarm_off(dev):
	"""Disables the Day-of-Week Clock alarm."""
	reg = dev.control_in(3, 0xA, 0, 1)
	reg = chr(ord(reg) & 0x7F)
	dev.control_out(4, 0xA, 0, reg)

def dow_freq_test(dev, enable):
	"""Enables or disables the Day-of-Week Clock frequency test."""
	dev.control_in(5, 0, enable, 0)

def dow_freq_out(dev, freq):
	"""Enables or disables the Day-of-Week Clock frequency test."""
	dev.control_in(6, 0, freq, 0)

def dow_calibrate(dev, ppm):
	"""Sets a new calibration value for the Day-of-Week Clock."""
	if ppm >= 0:
		dev.control_in(7, 1, int(math.floor(ppm / 4.068)), 0)
	else:
		dev.control_in(7, 0, int(math.floor(ppm / -2.034)), 0)

def dow_dump(dev):
	"""Dumps the entire Day-of-Week Clock memory."""
	usbtiny.dump(0x0, dev.control_in(3, 0, 0, 7) + dev.control_in(3, 0x7, 0, 7) + dev.control_in(3, 0xE, 0, 6))
	dev.control_in(3, 0x10, 0, 2) # move address pointer away from 0x0:0x7

def dow_restore_defaults(dev):
	"""Restores the factory defaults of the Day-of-Week Clock."""
	dev.control_in(8, 0x3B, 0xAA, 0)

vendor  = 0x6666
product = 0x0002

dev = usbtiny.USBtiny(vendor, product)
td = datetime.datetime.now() - dow_get_time(dev)
print (td.microseconds + (td.seconds + td.days * 24 * 3600) * 1000000.0) / 1000000.0
dow_dump(dev)