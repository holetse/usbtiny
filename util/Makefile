# ======================================================================
# Build a Python wrapper for libusb.
#
# Copyright 2006-2010 Dick Streefland
#
# This is free software, licensed under the terms of the GNU General
# Public License as published by the Free Software Foundation.
# ======================================================================

PYTHONINC=$(shell python -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc()")

all:		_libusb.so

clean:
	rm -f libusb.pyc libusb_wrap.c usbtiny.pyc

clobber:	clean
	rm -f _libusb.so libusb.py

_libusb.so:	libusb.i
	swig -python -D'__attribute__(x)=' -w302,454 -I/usr/include -I/usr/local/include/ $<
	gcc -bundle -fPIC -I$(PYTHONINC) -I/usr/local/include/ -o _libusb.so libusb_wrap.c -lpython -L/usr/local/lib/ -lusb
	@rm -f libusb_wrap.c
