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
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "usb.h"
#include "main.h"
#include "usbtiny.h"
#include "stddef.h"
#include "m41t81s.h"
#include <util/delay.h>

int execute_cmd = USBTINY_NO_CMD;
int out_address = 0;
unsigned int leds = 0;
volatile byte_t pcif = 0;

ISR(PCINT_vect, ISR_NAKED) {
	asm("push r24\n\t"
		"ldi r24, 0x01\n\t"
		"sts pcif, r24\n\t"
		"pop r24\n\t"
		: :
	);
	reti();
}

// ----------------------------------------------------------------------
// Handle a non-standard SETUP packet.
// ----------------------------------------------------------------------
extern	byte_t	usb_setup ( byte_t data[8] )
{
	byte_t value;
	execute_cmd = data[1];
	switch (data[1]) {
		case USBTINY_ECHO:
			execute_cmd = USBTINY_NO_CMD;
			return 8;
		case USBTINY_SET_TIME:
			return 0;
		case USBTINY_GET_TIME:
			clock_read(0x0, data, 8);
			/* Remove some control bits from time data */
			data[1] &= 0x7F;
			data[3] &= 0x7F;
			execute_cmd = USBTINY_NO_CMD;
			return 8;
		case USBTINY_CLOCK_RAW_READ:
			value = data[6];
			clock_read(data[2], data, value);
			execute_cmd = USBTINY_NO_CMD;
			return value;
		case USBTINY_CLOCK_RAW_WRITE:
			out_address = data[2];
			return 0;
		case USBTINY_FREQ_TEST:
			value = data[4];
			clock_freq_test(value);
			execute_cmd = USBTINY_NO_CMD;
			return 0;
		case USBTINY_FREQ_OUT:
			value = data[4];
			clock_freq_out(value);
			execute_cmd = USBTINY_NO_CMD;
			return 0;
		case USBTINY_CALIBRATE:
			value = data[4];
			clock_calibrate(value, data[2]);
			execute_cmd = USBTINY_NO_CMD;
			return 0;
		case USBTINY_FACTORY_DEFAULT:
			if (data[4] == USBTINY_RESTORE_UNLOCK_VALUE && data[2] == USBTINY_RESTORE_UNLOCK_INDEX) {
				clock_default_restore();
			}
			execute_cmd = USBTINY_NO_CMD;
			return 0;
	}
	execute_cmd = USBTINY_NO_CMD;
	return 0;
}

// ----------------------------------------------------------------------
// Handle an IN packet. (USBTINY_CALLBACK_IN==1)
// ----------------------------------------------------------------------
extern	byte_t	usb_in ( byte_t* data, byte_t len )
{
	return 0;
}

// ----------------------------------------------------------------------
// Handle an OUT packet. (USBTINY_CALLBACK_OUT==1)
// ----------------------------------------------------------------------
extern	void	usb_out ( byte_t* data, byte_t len )
{
	if (execute_cmd == USBTINY_SET_TIME) {
		clock_write(0, data, len);
		execute_cmd = USBTINY_NO_CMD;
	} else if (execute_cmd == USBTINY_CLOCK_RAW_WRITE) {
		clock_write(out_address, data, len);
		execute_cmd = USBTINY_NO_CMD;
	}
}

void shift_init() {
	CLR(SHIFT_SERIAL_PIN);
	OUTPUT(SHIFT_SERIAL_PIN);
	CLR(SHIFT_CLOCK_PIN);
	OUTPUT(SHIFT_CLOCK_PIN);
	SET(SHIFT_GATE_PIN);
	OUTPUT(SHIFT_GATE_PIN);
}

void shift_bits(byte_t bits) {
	byte_t i = 0;
	
	SET(SHIFT_GATE_PIN);
	for(i = 0; i < 8; ++i) {
		
		/* load serial out */
		
		if (bits & 0x80) {
			SET(SHIFT_SERIAL_PIN);
		} else {
			CLR(SHIFT_SERIAL_PIN);
		}
		
		/* pulse clock */
		
		SET(SHIFT_CLOCK_PIN);
		CLR(SHIFT_CLOCK_PIN);
		bits <<= 1;
	}
	
	/* pulse clock once more--shift and storage clocks are tied together */
	
	SET(SHIFT_CLOCK_PIN);
	CLR(SHIFT_CLOCK_PIN);
	
	CLR(SHIFT_GATE_PIN);
}

void update_leds() {
	byte_t bits = 0;
	
	/* rows */
	bits |= !!(leds & 0xF);
	bits |= !!(leds & 0xF0) << 1;
	bits |= !!(leds & 0xF00) << 2;
	bits |= !!(leds & 0xF000) << 3;
	
	/* cols */
	bits |= !(leds & 0x1111) << 4;
	bits |= !(leds & 0x2222) << 5;
	bits |= !(leds & 0x4444) << 6;
	bits |= !(leds & 0x8888) << 7;
	
	shift_bits(bits);
}

void setup_next_alarm() {
	byte regs[8], hour;
	const byte midnight[5] = {0x80, 0xC0, 0x00, 0x00, 0x00};
	const byte noon[5] = {0x80, 0xC0, 0x12, 0x00, 0x00};
	clock_read(0, regs, 8);
	hour = (((regs[3] & 0x30) >> 4) * 10) + (regs[3] & 0x0F);
	if (hour >= 12) {
		/* set alarm for 00:00:00  (match 0x18) */
		clock_write(0xA, midnight, sizeof(midnight));
		leds &= 0xC000;
		BIT_SET(leds, ((regs[4] - 1) * 2) + 1);
	} else {
		/* set alarm for 12:00:00  (match 0x18) */
		clock_write(0xA, noon, sizeof(noon));
		leds &= 0xC000;
		BIT_SET(leds, ((regs[4] - 1) * 2));
	}
	
	clock_read(0xF, &hour, 1); /* clear AF */
	update_leds();
}

void standby() {
	set_sleep_mode(SLEEP_MODE_PWR_SAVE);
	sleep_mode();
}

void power_down() {
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	sleep_mode();
}

void idle() {
	set_sleep_mode(SLEEP_MODE_IDLE);
	sleep_mode();
}

// ----------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------
extern	int	main ( void )
{
	byte_t last_connected = 0;
	power_all_disable();
	power_usi_enable();
	clock_init();
	shift_init();
	/* enable pull up */
	INPUT(ALARM_PIN);
	SET(ALARM_PIN);
	/* configure port change */
	PCMSK1 = 0x2;
	PCMSK0 = 0x10;
	BIT_SET(GIFR, PCIF);
	BIT_SET(GIMSK, PCIE0);
	BIT_SET(GIMSK, PCIE1);
	setup_next_alarm();
	
	usb_init();
	
	for	( ;; )
	{
		usb_poll();
		if (last_connected != usb_connected()) {
			last_connected = !last_connected;
			if (last_connected) {
				BIT_SET(leds, 15);
			} else {
				BIT_CLR(leds, 15);
			}
			update_leds();
		}
		if (pcif) {
			/* disable alarm pin interrupt while we set alarm */
			BIT_CLR(GIMSK, PCIE0);
			setup_next_alarm();
			BIT_SET(GIMSK, PCIE0);
			pcif = 0;
		}
		if (last_connected == 0 && !TST(USBTINY_USB_VBUS_SENSE)) {
			BIT_CLR(leds, 15);
			update_leds();
			power_down();
		}
	}
}
