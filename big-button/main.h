#ifndef _MAIN_H__
#define _MAIN_H__

#define RELAY_PORT B
#define RELAY_PIN 3
#define BUTTON_SENSE_PORT B
#define BUTTON_SENSE_PIN 6
#define BUTTON_LED_PORT A
#define BUTTON_LED_PIN 1

// Bit manipulation macros
#define	BIT_CLR(reg,bit)	{ (reg) &= ~ _BV(bit); }
#define	BIT_SET(reg,bit)	{ (reg) |=   _BV(bit); }
#define	BIT_TST(reg,bit)	(((reg) & _BV(bit)) != 0)

#define MAIN_H_CONCAT(a, b) (a ## b)

// I/O port manipulation macros
#define	DDR_CLR(p,b)		BIT_CLR(MAIN_H_CONCAT(DDR, p), b)
#define	DDR_SET(p,b)		BIT_SET(MAIN_H_CONCAT(DDR, p), b)
#define	PORT_CLR(p,b)		BIT_CLR(MAIN_H_CONCAT(PORT, p), b)
#define	PORT_SET(p,b)		BIT_SET(MAIN_H_CONCAT(PORT, p), b)
#define	PORT_TST(p,b)		BIT_TST(MAIN_H_CONCAT(PORT, p), b)
#define	PIN_TST(p,b)		BIT_TST(MAIN_H_CONCAT(PIN, p), b)
#define	PIN_SET(p,b)		BIT_SET(MAIN_H_CONCAT(PIN, p), b)

#endif
