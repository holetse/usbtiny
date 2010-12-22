#!/usr/bin/python
# ======================================================================
# template.py - USBtiny/template test program
#
# Copyright 2006-2010 Dick Streefland
# ======================================================================

import sys, os.path, time, getopt
sys.path[0] = os.path.join(sys.path[0], '../util')
import usbtiny

vendor  = 0x1781
product = 0x0C9F
powerup = 5
sck_period = 10
change_only = False
reset_state = 0

optlist, args = getopt.getopt(sys.argv[1:], "hl", ["high", "low"])


for opt, arg in optlist:
	if opt in ("--high", "-h"):
		reset_state = 1
		change_only = True
	elif opt in ("--low", "-l"):
		reset_state = 0
		change_only = True

dev = usbtiny.USBtiny(vendor, product)
if change_only:
	if reset_state is 1:
		reset_desc = 'HIGH'
	else:
		reset_desc = 'LOW'
	print 'Setting RESET to ' + reset_desc + '...'
	dev.control_in(powerup, sck_period, reset_state, 0)
else:
	print 'Resetting...'
	dev.control_in(powerup, sck_period, 0, 0)
	time.sleep(2)
	dev.control_in(powerup, sck_period, 1, 0)
print 'Done.'
