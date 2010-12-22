#ifndef _MAIN_H__
#define _MAIN_H__

#define USBTINY_NO_CMD -1
#define USBTINY_ECHO 0
#define USBTINY_GET_TIME 1
#define USBTINY_SET_TIME 2
#define USBTINY_CLOCK_RAW_READ 3
#define USBTINY_CLOCK_RAW_WRITE 4
#define USBTINY_FREQ_TEST 5
#define USBTINY_FREQ_OUT 6
#define USBTINY_CALIBRATE 7
#define USBTINY_FACTORY_DEFAULT 8

#define USBTINY_RESTORE_UNLOCK_VALUE 0xAA
#define USBTINY_RESTORE_UNLOCK_INDEX 0x3B

#define USBTINY_IN_TOKEN 0xFF

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
#define PIN_TST(p, b)		BIT_TST(PIN ## p, b)
#define	PIN_SET(p,b)		BIT_SET(PIN  ## p, b)

#define TST(a) PIN_TST a
#define SET(a) PORT_SET a
#define CLR(a) PORT_CLR a
#define INPUT(a) DDR_CLR a
#define OUTPUT(a) DDR_SET a

#define F_CPU 12000000UL

#define SHIFT_CLOCK_PIN (B, 3)
#define SHIFT_GATE_PIN (A, 3)
#define SHIFT_SERIAL_PIN (A, 5)
#define ALARM_PIN (B, 1)

#endif
