#ifndef _MAIN_H__
#define _MAIN_H__

#define USBTINY_ECHO 0
#define USBTINY_READ 1
#define USBTINY_WRITE 2

#define USBTINY_IN_TOKEN 0xFF

#define SPOOF_INIT_DELAY 12000
#define LED_PULSE_DELAY 5

// Bit manipulation macros
#define	BIT_CLR(reg,bit)	{ (reg) &= ~ _BV(bit); }
#define	BIT_SET(reg,bit)	{ (reg) |=   _BV(bit); }
#define	BIT_TST(reg,bit)	(((reg) & _BV(bit)) != 0)

// I/O port manipulation macros
#define	DDR_CLR(p,b)		BIT_CLR(DDR  ## p, b)
#define	DDR_SET(p,b)		BIT_SET(DDR  ## p, b)
#define	PORT_CLR(p,b)		BIT_CLR(PORT ## p, b)
#define	PORT_SET(p,b)		BIT_SET(PORT ## p, b)
#define	PORT_TST(p,b)		BIT_TST(PORT ## p, b)
#define	PIN_TST(p,b)		BIT_TST(PIN  ## p, b)
#define	PIN_SET(p,b)		BIT_SET(PIN  ## p, b)

#endif
