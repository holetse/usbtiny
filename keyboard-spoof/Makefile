# ======================================================================
# Template Makefile
#
# Copyright 2006-2010 Dick Streefland
#
# This is free software, licensed under the terms of the GNU General
# Public License as published by the Free Software Foundation.
# ======================================================================

USBTINY		= ../usbtiny
TARGET_ARCH	= -mmcu=attiny861
OBJECTS		= main.o
FLASH_CMD	= avrdude -P usb -c usbtiny -p ATtiny861 -U flash:w:main.hex
FUSES_CMD	= avrdude -P usb -c usbtiny -p ATtiny861 -U hfuse:w:0xdf:m -U lfuse:w:0xdf:m -U efuse:w:0x1:m
STACK		= 64
FLASH		= 8192
SRAM		= 512

include $(USBTINY)/common.mk
