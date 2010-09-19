// ======================================================================
// USBtiny template application
//
// Copyright 2006-2010 Dick Streefland
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

#include <avr/pgmspace.h>
#include "usb.h"
#include "main.h"

int usb_in_offset = 0;
int usb_out_offset = 0;
byte_t stored_data[32];
int store_data_len = 0;

// ----------------------------------------------------------------------
// Handle a non-standard SETUP packet.
// ----------------------------------------------------------------------
extern	byte_t	usb_setup ( byte_t data[8] )
{
	switch (data[1]) {
		case USBTINY_ECHO:
			return 8;
		case USBTINY_READ:
			usb_in_offset = data[4] + (data[5] << 8);
			return USBTINY_IN_TOKEN;
		case USBTINY_WRITE:
			usb_out_offset = data[4] + (data[5] << 8);
			if (usb_out_offset == 0) {
				store_data_len = 0;
			}
			return 0;
	}
	
	return 0;
}

// ----------------------------------------------------------------------
// Handle an IN packet. (USBTINY_CALLBACK_IN==1)
// ----------------------------------------------------------------------
extern	byte_t	usb_in ( byte_t* data, byte_t len )
{
	int i = 0;
	
	for(i = 0; (i < len) && (i + usb_in_offset < sizeof(stored_data)) && (i + usb_in_offset < store_data_len); ++i) {
		data[i] = stored_data[i + usb_in_offset];
	}
	
	return i;
}

// ----------------------------------------------------------------------
// Handle an OUT packet. (USBTINY_CALLBACK_OUT==1)
// ----------------------------------------------------------------------
extern	void	usb_out ( byte_t* data, byte_t len )
{
	int i = 0;
	
	for(i = 0; (i < len) && (i + usb_out_offset < sizeof(stored_data) && (store_data_len < sizeof(stored_data))); ++i) {
		stored_data[i + usb_out_offset] = data[i];
		store_data_len++;
	}
}

// ----------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------
extern	int	main ( void )
{
	usb_init();
	for	( ;; )
	{
		usb_poll();
	}
}
