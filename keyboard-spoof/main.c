// ======================================================================
// USBtiny template application
//
// Copyright 2006-2010 Dick Streefland
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

#include <string.h>
#include <avr/pgmspace.h>
#include "usb.h"
#include "main.h"
#include "usbtiny.h"
#include "stddef.h"

// HID Class Globals
typedef unsigned long counter_t;
byte_t keys[8] = {0, 0, 0, 0, 0, 0, 0, 0};
counter_t ms_counter = 0;
counter_t idle_counter = 0;
counter_t led_counter = 0;
byte_t idle_rate = 125;
byte_t keys_changed = 0;

byte_t	report_desc [USBTINY_HID_REPORT_DESCRIPTOR_LENGTH] PROGMEM = {
	0x05, 0x01, // Usage Page (Generic Desktop)
	0x09, 0x06, // Usage (Keyboard)
	0xA1, 0x01, // Collection (Application)
	0x05, 0x07, // Usage Page (Key Codes)
	0x19, 0xE0, // Usage Minimum (224)
	0x29, 0xE7, // Usage Maximum (231)
	0x15, 0x00, // Logical Minimum (0)
	0x25, 0x01, // Logical Maximum (1)
	0x75, 0x01, // Report Size (1)
	0x95, 0x08, // Report Count (8)
	0x81, 0x02, // Input (Data, Variable, Absolute) ;Modifier byte
	0x95, 0x01, // Report Count (1)
	0x75, 0x08, // Report Size (8)
	0x81, 0x01, // Input (Constant) ;Reserved byte
	0x95, 0x05, // Report Count (5)
	0x75, 0x01, // Report Size (1)
	0x05, 0x08, // Usage Page (LEDs)
	0x19, 0x01, // Usage Minimum (1)
	0x29, 0x05, // Usage Maximum (5)
	0x91, 0x02, // Output (Data, Variable, Absolute) ;LED report
	0x95, 0x01, // Report Count (1)
	0x75, 0x03, // Report Size (3)
	0x91, 0x01, // Output (Constant) ;LED report padding
	0x95, 0x06, // Report Count (6)
	0x75, 0x08, // Report Size (8)
	0x15, 0x00, // Logical Minimum (0)
	0x26, 0xFF, 0x00, // Logical Maximum(255)
	0x05, 0x07, // Usage Page (Key Codes)
	0x19, 0x00, // Usage Minimum (0)
	0x2A, 0xFF, 0x00, // Usage Maximum (255)
	0x81, 0x00, // Input (Data, Array) ;Key arrays (6 bytes)
	0xC0, // End Collection
};


// Spoofer Globals
typedef long spoof_delay_t;

typedef struct {
  spoof_delay_t delay; // how long to hold these keys down for
  byte_t keys[8]; // the keys
} spoof_event_entry;

typedef struct {
	const spoof_event_entry * events;
	unsigned int length;
} spoof;

static spoof_event_entry hello_world_spoof[] PROGMEM = {
	{ 25, {0x02, 0x0, 0x0B, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x08, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x0F, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x00, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x0F, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x12, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x2C, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x02, 0x0, 0x1A, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x12, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x15, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x0F, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 25, {0x0, 0x0, 0x07, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ 5000, {0x0, 0x0, 0x00, 0x0, 0x0, 0x0, 0x0, 0x0} },
	{ -1, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} }
};

static spoof spoofs[] PROGMEM = {
	{hello_world_spoof, sizeof(hello_world_spoof) / sizeof(spoof_event_entry)}
};


spoof_delay_t spoof_delay = SPOOF_INIT_DELAY;
unsigned int spoof_index = 0;
unsigned int spoofs_index = 0;


// ----------------------------------------------------------------------
// Handle a non-standard SETUP packet.
// ----------------------------------------------------------------------
extern	byte_t	usb_setup ( byte_t data[8] )
{	
	int i;
	static byte_t protocol = 0x1;
	
	switch (data[1]) {
		case 0x01: // GET_REPORT
			if (data[2] == 0x01) { // Input
				for(i = 0; i < sizeof(data); ++i) {
					data[i] = keys[i];
				}
				return sizeof(data);
			}
			
			break;
		case 0x02: // GET_IDLE
			data[0] = idle_rate;
			return 1;
		case 0x03: // GET_PROTOCOL
			data[0] = protocol;
			return 1;
		case 0x09: // SET_REPORT
			if (data[2] == 0x02) { // Output
				return 0;
			}
			
			break;
		case 0x0A: // SET_IDLE
			idle_rate = data[2];
			break;
		case 0x0B: // SET_PROTOCOL
			protocol = data[2] || data[3];
			break;
		default:
			break;
	}
	
	return 0;
}

// ----------------------------------------------------------------------
// Handle an IN packet. (USBTINY_CALLBACK_IN==1)
// ----------------------------------------------------------------------
extern	byte_t	usb_in ( byte_t* data, byte_t len )
{
	int i = 0;
	
	for(i = 0; i < len; ++i) {
		data[i] = keys[i];
	}
	
	return i;
}

// ----------------------------------------------------------------------
// Handle an OUT packet. (USBTINY_CALLBACK_OUT==1)
// ----------------------------------------------------------------------
extern	void	usb_out ( byte_t* data, byte_t len )
{

}

void timer0_init( void ) {
	TCCR0B = _BV(TSM) | _BV(PSR0);
	TCCR0B = _BV(TSM) | _BV(PSR0) | _BV(CS02);
	TCCR0A = _BV(0);
	TCNT0L = 0;
	OCR0A = 47; // ~ 1ms
	TIFR = _BV(OCF0A);
	TIMSK &= ~_BV(OCIE0A);
	TCCR0B &= ~_BV(TSM);
}

void timer0_poll( void ) {
	if(TIFR & _BV(OCF0A))
	{
		TIFR = _BV(OCF0A);
		ms_counter++;
		idle_counter++;
		led_counter++;
	}
}

void key_report_poll( void ) {
	
	if(keys_changed)
	{
		// Change keys
		usb_load_endp1(keys, sizeof(keys));
		idle_counter = 0;
		keys_changed = 0;
		DDR_SET(A, 1);
	}

	if(idle_rate && idle_counter >= (idle_rate * 4))
	{
		// Force a report
		usb_load_endp1(keys, sizeof(keys));
		idle_counter = 0;
	}
}

void spoofer_poll( void ) {
	int i;
	byte_t *src;
	unsigned int spoof_length;
	const spoof * spoof_desc;
	const spoof_event_entry * spoof_events;
	
	
	if(ms_counter >= spoof_delay && spoof_delay != -2) // loop forever on -2
	{
		spoof_desc = &spoofs[spoofs_index];
		spoof_events = (const spoof_event_entry *)pgm_read_word(spoof_desc + offsetof(spoof, events));
		spoof_length = pgm_read_word(((byte_t *) spoof_desc) + offsetof(spoof, length));
		spoof_delay = pgm_read_dword(&(spoof_events[spoof_index].delay));
		// should we skip to a new spoof description?
		if (spoof_delay == -1) { // yes
			spoof_delay = 1000; // random here!
			ms_counter = 0;
			spoof_index = 0;
			spoofs_index++; // random here!
			if (spoofs_index >= sizeof(spoofs) / sizeof(spoof)) {
				spoofs_index = 0;
			}
		} else { // no
			src = (byte_t *) spoof_events[spoof_index].keys;
			for(i = 0; i < sizeof(keys); ++i) {
				keys[i] = pgm_read_byte(src);
				src++;
			}
			ms_counter = 0;
			keys_changed = 1;
			spoof_index++;
			if (spoof_index >= spoof_length) {
				spoof_index = 0;
			}
		}
	}
}


// ----------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------
extern	int	main ( void )
{
	PORT_SET(A, 1);
	DDR_SET(A, 0);
	usb_init();
	timer0_init();
	for	( ;; )
	{
		usb_poll();
	    timer0_poll();
	    key_report_poll();
	    spoofer_poll();
	
		if (led_counter >= LED_PULSE_DELAY) {
			DDR_CLR(A, 1);
			led_counter = 0;
		}
	}
}
