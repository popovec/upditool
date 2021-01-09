/*
    UPDI_ll.c

    This is part of c_updi, programmer

    Copyright (C) 2020 Peter Popovec, popovec.peter@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    UPDI low level - serial port (TTL USB serial converter i.e CH340, CP2102
    (posix code, but for linux termios2 is used)

*/
#if defined(WIN32) || defined(WIN64)

#include "UPDI_ll_win.c"
#else
// linux specifix or posix

#ifdef __linux__
#define USE_TEMIOS2
#endif
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef USE_TEMIOS2
#include <asm/ioctls.h>
#include <asm/termbits.h>
#else
#include <termios.h>
#endif
#include <unistd.h>
#include <sys/file.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <time.h>
#include "UPDI_ll.h"

//#define DEBUG(msg...) printf(msg)
#define DEBUG(msg...)
struct sparam {
	char *device;
	int fd;
#ifdef USE_TEMIOS2
	struct termios2 ts;
#else
	struct termios ts;
#endif
} sparam;

void updi_serial_close()
{
	if (sparam.fd >= 0)
		close(sparam.fd);
}

int updi_serial_init(char *serial)
{
	sparam.device = strdup(serial);

	//use O_NDELAY to ignore DCD .. seems to be same as CLOCAL ?
	sparam.fd = open(sparam.device, O_RDWR | O_NONBLOCK | O_NOCTTY | O_NDELAY);

	if (sparam.fd == -1) {
		printf("Unable to open %s, %s\n", sparam.device, strerror(errno));
		return (1);
	}
	if (flock(sparam.fd, LOCK_EX | LOCK_NB) != 0) {
		printf("Unable to lock %s, %s\n", sparam.device, strerror(errno));
		return (1);
	}
#ifndef USE_TEMIOS2
	if (tcflush(sparam.fd, TCIOFLUSH) != 0) {
		printf("Unable to flush %s, %s\n", sparam.device, strerror(errno));
		return (1);
	}
#endif

#ifdef USE_TEMIOS2
	if (ioctl(sparam.fd, TCGETS2, &sparam.ts) < 0) {
		printf("ioctl TCGETS2\n");
		return -2;
	}
#else
//  do not use ioclt, for posix system use tcgetattr
//  if (ioctl (sparam.fd, TCGETS, &sparam.ts) != 0)
	if (tcgetattr(sparam.fd, &sparam.ts) != 0) {
		printf("Unable to get line parameters %s, %s\n", sparam.device, strerror(errno));
		return (1);
	}
#endif

/* ************************************************************************************ */

	//default
	sparam.ts.c_cflag = 0;

	//set control flags (use only POSIX declared values!!!)

	sparam.ts.c_cflag &= ~CSIZE;	// clear character size bits
	sparam.ts.c_cflag |= CS8;	// set 8 bits per character
	sparam.ts.c_cflag |= CSTOPB;	// two stop bit
	sparam.ts.c_cflag &= ~PARODD;	// even parity
	sparam.ts.c_cflag |= PARENB;	// parity
	sparam.ts.c_cflag |= CREAD;	// enable receiver
	sparam.ts.c_cflag |= CLOCAL;	// Ignore modem control lines.
	sparam.ts.c_cflag |= HUPCL;	// hang up after close (lower modem control lines)
#ifdef USE_TEMIOS2
	sparam.ts.c_cflag &= ~CBAUD;
	sparam.ts.c_cflag |= BOTHER;
	sparam.ts.c_ispeed = 115200;
	sparam.ts.c_ospeed = 115200;
#else
	cfsetspeed(&sparam.ts, B115200);	// after BREAK  UPDI clock = 4MHz -> 115200 is OK
#endif
	// disable all input processing
	sparam.ts.c_iflag = 0;
	//disable all output processing
	sparam.ts.c_oflag = 0;
	//disable all local modes
	sparam.ts.c_lflag = 0;

/* ************************************************************************************ */

#ifdef  USE_TEMIOS2
	if (ioctl(sparam.fd, TCSETS2, &sparam.ts) < 0) {
		printf("ioctl TCSETS2\n");
		return -3;
	}
	if (ioctl(sparam.fd, TCGETS2, &sparam.ts) < 0) {
		printf("ioctl TCGETS2\n");
		return -2;
	}
#else
	//change immediately all parameters
	if (tcsetattr(sparam.fd, TCSANOW, &sparam.ts))
		return 1;
#endif
	//alternate call ioctl(sparam.fd, TCSETS, &sparam.ts)
/* ************************************************************************************ */

// this seems to be alternative to VTIME=0 and VMIN=0
// fcntl(sparam.fd, F_SETFL, FNDELAY);             /* read return 0  if no character is available */
#ifndef USE_TEMIOS2
	if (tcflush(sparam.fd, TCIOFLUSH) != 0) {
		printf("Unable to flush %s, %s\n", sparam.device, strerror(errno));
		return 1;
	}
#endif
	return 0;
}

static uint8_t updi_serial_avail(int timeout)
{
	fd_set rfds;
	struct timeval tv;
	int retval;

	FD_ZERO(&rfds);
	FD_SET(sparam.fd, &rfds);

	tv.tv_sec = 0;
	tv.tv_usec = timeout;

	retval = select(sparam.fd + 1, &rfds, NULL, NULL, &tv);
	if (retval != 0)
		return 1;
	return 0;
}

// 0 = OK
static int updi_serial_rx_clear(int count)
{
	int c;

	while (count--) {
		if (updi_serial_avail(4999) == 0)
			return 0;
		read(sparam.fd, &c, 1);
	}
	return 1;
}

int updi_set_speed(int speed)
{
#ifdef USE_TEMIOS2
	struct termios2 ts;

	sparam.ts.c_cflag &= ~CBAUD;
	sparam.ts.c_cflag |= BOTHER;
	sparam.ts.c_ispeed = speed;
	sparam.ts.c_ospeed = speed;

	if (ioctl(sparam.fd, TCSETS2, &sparam.ts) < 0) {
		printf("ioctl TCSETS2\n");
		return -3;
	}
	if (ioctl(sparam.fd, TCGETS2, &ts) < 0) {
		printf("ioctl TCGETS2\n");
		return -2;
	}
	if (ts.c_ispeed != speed) {
		fprintf(stderr, "Unable to set speed, (requested %d achieved %d)\n",
			speed, ts.c_ispeed);
		return 1;
	}
	return 0;
#else
	struct termios ts;
	int Bs;

	if (speed == 300)
		Bs = B300;
	else if (speed == 115200)
		Bs = B115200;
	else if (speed == 230400)
		Bs = B230400;
	else if (speed == 460800)
		Bs = B460800;
	else if (speed == 500000)
		Bs = B500000;
	else if (speed == 576000)
		Bs = B576000;
	else
		return 1;

	if (cfsetspeed(&sparam.ts, Bs))
		return 1;
	if (tcsetattr(sparam.fd, TCSAFLUSH, &sparam.ts))
		return 1;
	usleep(5000);
	if (tcgetattr(sparam.fd, &ts))
		return 1;

	if (cfgetispeed(&ts) != cfgetispeed(&sparam.ts))
		return 1;
	if (cfgetospeed(&ts) != cfgetospeed(&sparam.ts))
		return 1;
	return 0;
#endif

}

int updi_send_break()
{
	int ret;
	uint8_t b[2] = { 0, 0 };

	DEBUG("[%s] called\n", __FUNCTION__);
// minimal 24.6ms (atmega4808 doc, table 30-3
// send frame start bit, 8 bit data, 1 parity = 10 bits at level 0
// at baud rate 300 => 30mS BEAK

// atmega4808 doc chapter 30.3.1.2 - debugger shoud send two break's
	ret = updi_set_speed(300);
	if (ret)
		return 1;

// TCSAFLUSH is sometime wrong on some USB to serial addapters, clear RX
	if (updi_serial_rx_clear(100)) {
		printf("%s Someting is wrong, unable to clear receive buffer\n", __FUNCTION__);
		return 1;
	}
// send double break 
	write(sparam.fd, b, 2);

// tcdrain is sometime wrong on USB serial adapter (and flag TCSADRAIN is unusable in tcsetattr)
// wait 24 bits at B300: 24*1000000/300 = 80000
	usleep(80000);
#ifndef USE_TEMIOS2
	tcdrain(sparam.fd);
#endif
// clear RX 
	if (updi_serial_rx_clear(100)) {
		printf("%s Someting is wrong, unable to clear receive buffer\n", __FUNCTION__);
		return 1;
	}
// after BREAK UPDI clock is set to 4MHz, speed 0.75 .. 150kbit .. 
	ret = updi_set_speed(115200);
	if (ret)
		return 1;

	usleep(1000);
	DEBUG("[%s] OK\n", __FUNCTION__);
	return 0;
}

int updi_transaction(uint8_t * send, uint16_t s_len, uint8_t * receive, uint16_t r_len, int timeout)
{
	uint16_t rr_len = 0;
	uint8_t c;
	struct timespec t_start, t_stop;
	long tout;

	int i;

	DEBUG("[%s] called, send %d chars/receive max %d chars \n", __FUNCTION__, s_len, r_len);

	DEBUG("[%s] transmit: ", __FUNCTION__);
	for (i = 0; i < s_len; i++)
		DEBUG("%02x ", send[i]);
	DEBUG("\n");
	write(sparam.fd, send, s_len);

	DEBUG("[%s] receiving: ", __FUNCTION__);
	i = 0;
	while (r_len--) {
		if (clock_gettime(CLOCK_MONOTONIC, &t_start))
			return 0;

		if (r_len > 0)
			tout = timeout;

		if (updi_serial_avail(tout) == 0) {
			DEBUG("\n[%s] no more chars (timeout)\n", __FUNCTION__);
			return rr_len;
		}
		if (clock_gettime(CLOCK_MONOTONIC, &t_stop))
			return 0;

		// adaptive timeout (from last received char)
		tout = t_stop.tv_sec - t_start.tv_sec;
		tout *= 1000000000;
		tout += t_stop.tv_nsec - t_start.tv_nsec;
		tout *= 15;
		tout /= 10000;
		if (tout > timeout)
			tout = timeout;

		read(sparam.fd, &c, 1);
		DEBUG("%02x ", c);

		receive[i] = c;
		if (i < s_len)
			if (send[i] != receive[i]) {
				DEBUG("\n[%s] RX does not match TX %02x %02x \n", __FUNCTION__,
				      receive[i], send[i]);
				return 0;
			}
		i++;
		rr_len++;
	}
	DEBUG("\n[%s] out of buffer\n", __FUNCTION__);
	return rr_len;
}
#endif				// end of posix/linux code
