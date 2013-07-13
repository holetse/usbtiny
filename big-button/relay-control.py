#!/usr/bin/python
# ======================================================================
# relay-control.py: an example showing how to control the BigButton
#                   relay interface.
#
# Copyright 2013 Edward Holets
# ======================================================================

# The relay is controlled via a USB control request of type 0x0C. The
# value argument is used to select the control mode. The index argument
# is used in self-timed mode to send the on duration (signed 16 bits, 
# in milliseconds). The control transfer direction is ignored, and 0
# bytes will be returned for IN transfers.
#
# Control Modes:
#   0x00: Relay Off
#   0x01: Relay On
#   0x02: Relay Self-Timed

import sys, os.path, time
import usb1, libusb1

RELAY_OFF = 0x00
RELAY_ON = 0x01
RELAY_TIMED = 0x02
RELAY_CONTROL_REQUEST = 0x0C

def relay_control(relay_handle, mode, duration=0):
  relay_handle.controlWrite(libusb1.LIBUSB_TYPE_VENDOR | libusb1.LIBUSB_RECIPIENT_DEVICE, RELAY_CONTROL_REQUEST, mode, duration, "", 500)

BUTTON_VENDOR  = 0x6666
BUTTON_PRODUCT = 0x0003

try:
  print "(Ctrl-C to exit)"

  context = usb1.USBContext()
  handle = context.getByVendorIDAndProductID(BUTTON_VENDOR, BUTTON_PRODUCT).open()

  print "turn the relay on"
  relay_control(handle, RELAY_ON)
  time.sleep(5)

  print "turn the relay off"
  relay_control(handle, RELAY_OFF)
  time.sleep(5)

  print "self timed 1000ms on"
  while 1:
    now = time.time()
    relay_control(handle, RELAY_TIMED, 1000)
    time.sleep(max((now + 5) - time.time(), 0))
except KeyboardInterrupt:
  pass

if handle:
  handle.close()
context.exit()