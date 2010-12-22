#include "m41t81s.h"
#include "main.h"
#include <avr/io.h>
#include <util/delay.h>

#define SCK_PIN 2
#define SDA_PIN 0

#define i2c_delay() _delay_us(8)
#define i2c_clk_pulse() BIT_SET(USICR, USITC); i2c_delay(); BIT_SET(USICR, USITC); BIT_SET(USICR, USICLK)
#define i2c_clk_read(var) BIT_SET(USICR, USITC); i2c_delay(); BIT_SET(USICR, USICLK); var = USIDR & 0x01; BIT_SET(USICR, USITC)
#define i2c_start() BIT_CLR(USIDR, 7); i2c_delay(); PORT_CLR(A, SCK_PIN); BIT_SET(USISR, USISIF)
#define i2c_restart() BIT_SET(USIDR, 7); i2c_delay(); PORT_SET(A, SCK_PIN); i2c_delay(); BIT_CLR(USIDR, 7); i2c_delay(); PORT_CLR(A, SCK_PIN); BIT_SET(USISR, USISIF)
#define i2c_stop() PORT_SET(A, SCK_PIN); i2c_delay(); BIT_SET(USIDR, 7)
#define i2c_nack() BIT_SET(USIDR, 7); BIT_SET(USICR, USITC) i2c_delay(); BIT_SET(USICR, USITC)
#define i2c_ack() BIT_CLR(USIDR, 7); BIT_SET(USICR, USITC) i2c_delay(); BIT_SET(USICR, USITC)

void i2c_tx(byte out) {
	byte i;
	
	USIDR = out;
	i2c_delay();
	for(i = 0; i < sizeof(byte) * 8; ++i) {
		i2c_clk_pulse();
		i2c_delay();
	}
}

byte i2c_rx() {
	byte i, in = 0, bit;
	
	DDR_CLR(A, SDA_PIN);
	i2c_delay();
	for(i = 0; i < sizeof(byte) * 8; ++i) {
		i2c_clk_read(bit);
		in <<= 1;
		in += bit;
		i2c_delay();
	}
	
	DDR_SET(A, SDA_PIN);
	
	return in;
}

byte i2c_read_ack() {
	byte ack = 0;
	DDR_CLR(A, SDA_PIN);
	i2c_clk_read(ack);
	DDR_SET(A, SDA_PIN);
	i2c_delay();
	return ack;
}

byte i2c_address(byte addr, byte read) {
	i2c_delay();
	i2c_tx((addr << 1) + (read != 0));
	
	if (i2c_read_ack()) {
		/* address not responding */
		return 1;
	}
	
	return 0;
}

void i2c_busy_detect() {
	byte data_line = 0;
	byte clock_line = 0;
	byte i;
	
	DDR_CLR(A, SDA_PIN);
	DDR_CLR(A, SCK_PIN);
	for(i = 0; i < 10; ++i) {
		while (!clock_line) {
			clock_line = TST((A, SCK_PIN));
			i2c_delay();
		}
		while (!data_line) {
			BIT_SET(USICR, USICLK);
			data_line = USIDR & 0x01;
			i2c_delay();
		}
		i2c_delay();
	}
	DDR_SET(A, SDA_PIN);
	DDR_SET(A, SCK_PIN);
}

void i2c_tx_detect() {
	byte data_line = 0;
	
	DDR_CLR(A, SDA_PIN);
	data_line = USIDR & 0x01;
	if (!data_line) {
		PORT_CLR(A, SCK_PIN);
		i2c_delay();
		data_line = USIDR & 0x01;
		while (!data_line) {
			i2c_clk_pulse();
			data_line = USIDR & 0x01;
			i2c_delay();
		}
		PORT_SET(A, SCK_PIN);
		i2c_delay();
	}
	DDR_SET(A, SDA_PIN);
}

void i2c_init() {
	BIT_SET(USIPP, USIPOS); /* use PA2:PA0 for SCL:SDA */
	PORT_SET(A, SCK_PIN);
	DDR_SET(A, SCK_PIN);
	PORT_SET(A, SDA_PIN);
	DDR_SET(A, SDA_PIN);
	BIT_SET(USIDR, 7);
	USICR = 0x20;
	i2c_tx_detect();
}

void clock_read(byte address, byte * buffer, int len) {
	unsigned int i;
	
	i2c_start();
	i2c_address(0x68, 0);
	i2c_tx(address);
	if (i2c_read_ack()) {
		/* tx failed */
		i2c_stop();
		return;
	}
	i2c_restart();
	i2c_address(0x68, 1);
	for(i = 0; i < len; ++i) {
		buffer[i] = i2c_rx();
		if (i == len - 1) {
			i2c_nack();
		} else {
			i2c_ack();
		}
		i2c_delay();
	}
	i2c_stop();
}

void clock_write(byte address, const byte * buffer, int len) {
	unsigned int i;
	
	i2c_start();
	i2c_address(0x68, 0);
	i2c_tx(address);
	if (i2c_read_ack()) {
		/* tx failed */
		i2c_stop();
		return;
	}
	for(i = 0; i < len; ++i) {
		i2c_tx(buffer[i]);
		if (i2c_read_ack()) {
			/* tx failed */
			i2c_stop();
			return;
		}
	}
	i2c_stop();
}

void clock_calibrate(byte value, byte sign) {
	byte reg;
	clock_read(0x8, &reg, 1);
	reg &= 0xC0;
	reg |= (!!sign) << 5;
	reg |= value & 0x1F;
	clock_write(0x8, &reg, 1);
}

void clock_freq_test(int enable) {
	byte reg;
	clock_read(0x8, &reg, 1);
	if (enable) {
		reg |= 0x40;
	} else {
		reg &= 0xBF;
	}
	clock_write(0x8, &reg, 1);
}

void clock_freq_out(byte freq) {
	byte reg;
	clock_read(0xA, &reg, 1);
	freq <<= 4;
	clock_write(0x13, &freq, 1);
	if (freq) {
		reg |= 0x40;
	} else {
		reg &= 0xBF;
	}
	clock_write(0xA, &reg, 1);
}

void clock_default_restore() {
	const byte registers[20] = {0, 0, 0, 0x80, 0x06, 0x01, 0x01, 0, 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	byte reg;
	clock_write(0, registers, 20);
	clock_read(0x8, &reg, 1);
}

void clock_init() {
	byte registers[20], reg;
	i2c_init();
	
	_delay_us(100);
	
	clock_read(0, registers, 20);
	
	if (registers[0x0F] & 0x04) {
		/* the oscilator failed, we need to initailize */
		registers[0x00] = 0;
		registers[0x01] = 0x80;
		registers[0x02] = 0;
		registers[0x03] = 0x80;
		registers[0x04] = 0x06;
		registers[0x05] = 1;
		registers[0x06] = 1;
		registers[0x07] = 0;
		registers[0x08] = 0x80;
		registers[0x09] = 0;
		registers[0x0A] = 0;
		registers[0x0B] = 0x40;
		registers[0x0C] = 0;
		registers[0x0D] = 0;
		registers[0x0E] = 0;
		registers[0x0F] = 0;
		registers[0x10] = 0;
		registers[0x12] = 0;
		registers[0x13] = 0;
		clock_write(0, registers, 20);
		
		/* restart clock */
		reg = 0;
		clock_write(0x01, &reg, 1);
		
		_delay_ms(4500);
		/* try and clear OF */
		reg = 0;
		clock_write(0x0F, &reg, 1);
	}
	
	clock_read(0x1, &reg, 1);
	if (reg & 0x80) {
		reg = 0;
		clock_write(0x01, &reg, 1);
	}
	
	/* disable output */
	clock_read(0x8, &reg, 1);
	if (!(reg & 0x80)) {
		reg |= 0x80;
		clock_write(0x8, &reg, 1);
	}
	
	/* clear update halt */
	clock_read(0xC, &reg, 1);
	reg &= 0xBF;
	clock_write(0x0C, &reg, 1);
}
