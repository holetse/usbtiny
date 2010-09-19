#!/usr/bin/python
# ======================================================================
# template.py - USBtiny/template test program
#
# Copyright 2006-2010 Dick Streefland
# ======================================================================

import sys, os.path, time
sys.path[0] = os.path.join(sys.path[0], '../util')
import usbtiny

vendor  = 0x6666
product = 0x0001

def control_in(device, request, val, index, size):
	response = str()
	requested = 0
	while requested < size:
		length = min(size - requested, 8)
		packet = device.control_in(request, val, index + requested, length)
		response = response + packet
		if len(packet) < length:
			break
		requested = requested + length
	return response
def control_out(device, request, val, index, data):
	sent = 0
	size = len(data)
	while sent < size:
		length = min(size - sent, 8)
		wrote = device.control_out(request, val, index + sent, data[sent:sent + length])
		if wrote < length:
			break
		sent = sent + length
	return sent

dev = usbtiny.USBtiny(vendor, product)
while 1:
	now = time.time()
	control_out(dev, 2, 0, 0, time.strftime('%Y-%m-%d %H:%M:%S'))
	print control_in(dev, 1, 0, 0, 32)
	time.sleep(max((now + 1) - time.time(), 0))