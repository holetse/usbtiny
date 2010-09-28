#!/usr/bin/python
# ======================================================================
# keyboard-config.py - Keyboard Spoofer spoof generator
#
# Copyright 2010 Edward Holets
# ======================================================================

import sys

spoof = ""
lookup_table = {
	'a': 0x04,
	'b': 0x05,
	'c': 0x06,
	'd': 0x07,
	'e': 0x08,
	'f': 0x09,
	'g': 0x0A,
	'h': 0x0B,
	'i': 0x0C,
	'j': 0x0D,
	'k': 0x0E,
	'l': 0x0F,
	'm': 0x10,
	'n': 0x11,
	'o': 0x12,
	'p': 0x13,
	'q': 0x14,
	'r': 0x15,
	's': 0x16,
	't': 0x17,
	'u': 0x18,
	'v': 0x19,
	'w': 0x1A,
	'x': 0x1B,
	'y': 0x1C,
	'z': 0x1D,
	'1': 0x1E,
	'2': 0x1F,
	'3': 0x20,
	'4': 0x21,
	'5': 0x22,
	'6': 0x23,
	'7': 0x24,
	'8': 0x25,
	'9': 0x26,
	'0': 0x27,
	'\n': 0x28,
	'\t': 0x2B,
	' ': 0x2C,
	'-': 0x2D,
	'=': 0x2E,
	'[': 0x2F,
	']': 0x30,
	'\\': 0x31,
	';': 0x33,
	'\'': 0x34,
	'`': 0x35,
	',': 0x36,
	'.': 0x37,
	'/': 0x38
}

shift_lookup_table = {
	'A': 0x04,
	'B': 0x05,
	'C': 0x06,
	'D': 0x07,
	'E': 0x08,
	'F': 0x09,
	'G': 0x0A,
	'H': 0x0B,
	'I': 0x0C,
	'J': 0x0D,
	'K': 0x0E,
	'L': 0x0F,
	'M': 0x10,
	'N': 0x11,
	'O': 0x12,
	'P': 0x13,
	'Q': 0x14,
	'R': 0x15,
	'S': 0x16,
	'T': 0x17,
	'U': 0x18,
	'V': 0x19,
	'W': 0x1A,
	'X': 0x1B,
	'Y': 0x1C,
	'Z': 0x1D,
	'!': 0x1E,
	'@': 0x1F,
	'#': 0x20,
	'$': 0x21,
	'%': 0x22,
	'^': 0x23,
	'&': 0x24,
	'*': 0x25,
	'(': 0x26,
	')': 0x27,
	'_': 0x2D,
	'+': 0x2E,
	'{': 0x2F,
	'}': 0x30,
	'|': 0x31,
	':': 0x33,
	'"': 0x34,
	'~': 0x35,
	'<': 0x36,
	'>': 0x37,
	'?': 0x38,
}

def build_hid_packet(c, last_c, delay):
	"""Builds a USB HID Boot keyboard IN payload for the
	character c based on the last character last_c. If there
	was no last character, pass None. Delay should be a time
	in ms to hold the keys down for."""
	if c in lookup_table:
		shift = 0
		code = lookup_table[c]
	elif c in shift_lookup_table:
		shift = 2
		code = shift_lookup_table[c]
	else:
		shift = 2
		code = shift_lookup_table['?']
	packet = ''
	if c == ' ':
		cname = '<space>'
	else:
		cname = c
	if last_c == c:
		packet = packet + "{ %d, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} }, // <nop>\n" % delay
	packet = packet + "{ %d, {0x%x, 0x0, 0x%02x, 0x0, 0x0, 0x0, 0x0, 0x0} }, // %s" % (delay, shift, code, cname)
	return packet

for line in sys.argv[1:len(sys.argv)]:
	oldc = None
	for c in line:
		print build_hid_packet(c, oldc, 25)
		oldc = c

