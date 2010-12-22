#ifndef _M41T81S_H__
#define _M41T81S_H__

typedef unsigned char byte;

void clock_init();
void clock_read(byte address, byte * buffer, int len);
void clock_write(byte address, const byte * buffer, int len);
void clock_freq_test(int enable);
void clock_freq_out(byte freq);
void clock_calibrate(byte value, byte sign);
void clock_default_restore();

#endif
