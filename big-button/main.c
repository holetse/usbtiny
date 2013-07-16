// ======================================================================
// USBtiny template application
//
// Copyright 2006-2010 Dick Streefland
//
// This is free software, licensed under the terms of the GNU General
// Public License as published by the Free Software Foundation.
// ======================================================================

#include <string.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include "usb.h"
#include "main.h"
#include "usbtiny.h"
#include "stddef.h"

#define RELAY_DURATION_NOT_SET -1
#define LED_DURATION_NOT_SET -1

// HID Class Globals
typedef unsigned int counter_t;
byte_t keys[8] = {0, 0, 0, 0, 0, 0, 0, 0};
counter_t ms_counter = 0;
counter_t idle_counter = 0;
counter_t relay_counter = 0;
counter_t led_counter = 0;
byte_t idle_rate = 125;
byte_t keys_changed = 0;
byte_t button_press_running = 0;
int relay_duration = RELAY_DURATION_NOT_SET;
int led_duration = LED_DURATION_NOT_SET;

const byte_t	report_desc [USBTINY_HID_REPORT_DESCRIPTOR_LENGTH] PROGMEM = {
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


// Key Event Globals
typedef long key_delay_t;

typedef struct {
  key_delay_t delay; // how long to hold these keys down for
  byte_t keys[8]; // the keys
} key_event_entry;

#define KEY_EVENT_EOL_DELAY -1

const static key_event_entry button_press_events[] PROGMEM = {
	{ 25, {0x0, 0x0, 0x2c, 0x0, 0x0, 0x0, 0x0, 0x0} }, // <space>
	{ 400, {0x0, 0x0, 0x00, 0x0, 0x0, 0x0, 0x0, 0x0} }, // <nop> .. long delay to debounce the button
	{ KEY_EVENT_EOL_DELAY, {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0} } // <end>
};

key_delay_t key_delay = KEY_EVENT_EOL_DELAY;
unsigned int key_event_index = 0;


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
				for(i = 0; i < 8; ++i) {
					data[i] = keys[i];
				}
				return 8;
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
		case 0x0C: // CONTROL RELAY
			switch (data[2]) {
				case 0x00: // off
					PORT_CLR(RELAY_PORT, RELAY_PIN);
					break;
				case 0x01: // on
					PORT_SET(RELAY_PORT, RELAY_PIN);
					break;
				case 0x02: // self timed
					PORT_SET(RELAY_PORT, RELAY_PIN);
					relay_duration = data[4] + (data[5] << 8); // duration in ms 
					break;
			}
			break;
		case 0x0D: // CONTROL LED
			switch (data[2]) {
				case 0x00: // off
					PORT_CLR(BUTTON_LED_PORT, BUTTON_LED_PIN);
					DDR_CLR(BUTTON_LED_PORT, BUTTON_LED_PIN);
					break;
				case 0x01: // on
					DDR_SET(BUTTON_LED_PORT, BUTTON_LED_PIN);
					PORT_SET(BUTTON_LED_PORT, BUTTON_LED_PIN);
					break;
				case 0x02: // self timed
					DDR_SET(BUTTON_LED_PORT, BUTTON_LED_PIN);
					PORT_SET(BUTTON_LED_PORT, BUTTON_LED_PIN);
					led_duration = data[4] + (data[5] << 8); // duration in ms 
					break;
			}
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
		relay_counter++;
		led_counter++;
	}
}

void button_init() {
	// button uses INT0 with a week pull-up
	// usb_init configures the external interrupts
	// to trigger on the falling edge, which is
	// what the button wants too.
	DDR_CLR(BUTTON_SENSE_PORT, BUTTON_SENSE_PIN);
	PORT_SET(BUTTON_SENSE_PORT, BUTTON_SENSE_PIN);
	PORT_CLR(BUTTON_LED_PORT, BUTTON_LED_PIN);
	DDR_SET(BUTTON_LED_PORT, BUTTON_LED_PIN);
	GIMSK &= ~(_BV(INTF0)); // disable INTO Interrupt Vector
	GIFR |= _BV(INTF0); // make sure INTF0 is clear
}

void button_stop() {
	button_press_running = 0;
}

void button_poll() {
	if (bit_is_set(GIFR, INTF0)) {
		GIFR |= _BV(INTF0); // clear it
		if (!button_press_running)
		{
			button_press_running = 1;
		}
	}
	if (led_duration != LED_DURATION_NOT_SET)
	{
		if (led_counter >= led_duration)
		{
			PORT_CLR(BUTTON_LED_PORT, BUTTON_LED_PIN);
			DDR_CLR(BUTTON_LED_PORT, BUTTON_LED_PIN);
			led_duration = LED_DURATION_NOT_SET;
		}
	} else {
		led_counter = 0;
	}
}

void keys_report_poll( void ) {
	
	if(keys_changed)
	{
		// Change keys
		usb_load_endp1(keys, sizeof(keys));
		idle_counter = 0;
		keys_changed = 0;
	}

	if(idle_rate && idle_counter >= (idle_rate * 4))
	{
		// Force a report
		usb_load_endp1(keys, sizeof(keys));
		idle_counter = 0;
	}
}

void keys_update_poll( void ) {
	int i;
	const byte_t *src;
	
	if (button_press_running)
	{
		if(ms_counter >= key_delay || key_delay == KEY_EVENT_EOL_DELAY)
		{
			key_delay = pgm_read_dword(&(button_press_events[key_event_index].delay));
			// are we done with our events?
			if (key_delay == KEY_EVENT_EOL_DELAY) { // yes
				ms_counter = 0;
				key_event_index = 0;
				button_stop();
			} else { // no
				src = button_press_events[key_event_index].keys;
				for(i = 0; i < sizeof(keys); ++i) {
					keys[i] = pgm_read_byte(src);
					src++;
				}
				ms_counter = 0;
				keys_changed = 1;
				key_event_index++;
			}
		}
	} else {
		ms_counter = 0;
	}
}

void relay_init() {
	PORT_CLR(RELAY_PORT, RELAY_PIN);
	DDR_SET(RELAY_PORT, RELAY_PIN);
}

void relay_poll() {
	if (relay_duration != RELAY_DURATION_NOT_SET)
	{
		if (relay_counter >= relay_duration)
		{
			PORT_CLR(RELAY_PORT, RELAY_PIN);
			relay_duration = RELAY_DURATION_NOT_SET;
		}
	} else {
		relay_counter = 0;
	}
}


// ----------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------
extern	int	main ( void )
{
	relay_init();
	usb_init();
	timer0_init();
	button_init();
	while (1)
	{
		usb_poll();
	  timer0_poll();
	  keys_report_poll();
	  keys_update_poll();
	  button_poll();
	  relay_poll();
	}
}
