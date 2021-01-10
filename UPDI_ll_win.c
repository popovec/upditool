/*
    UPDI_ll_win.c

    This is part of c_updi, programmer

    Copyright (C) 2021 Peter Popovec, popovec.peter@gmail.com

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
    windows driver

*/
/* windows code based on
 * https://www.xanthium.in/Serial-Port-Programming-using-Win32-API
 */

#include<windows.h>
#include<stdio.h>
#include <stdint.h>
#include <math.h>
HANDLE hComm;
DCB dcbSerialParams;
COMMTIMEOUTS ctmo;
int updi_speed;

/*@	\brief set serial speed
 *
 *	\param[int] speed  serial speed (115200.. 230400
 *	\return 	   0 error,
 *                         1 OK
 */

int updi_set_speed(int speed)
{
	uint8_t multiplier;
	switch (speed) {
	case (115200):
		dcbSerialParams.BaudRate = CBR_115200;
		break;
	case (230400):
		dcbSerialParams.BaudRate = 230400;
		break;
	default:
		fprintf(stderr, "Unsupported speed\n");
		return 1;
	}
	if (0 == SetCommState(hComm, &dcbSerialParams))
		return 1;

	updi_speed = speed;
	multiplier = (uint8_t) ceil((float)100000 / speed);
	ctmo.ReadIntervalTimeout = 20 * multiplier;
	ctmo.ReadTotalTimeoutMultiplier = 1 * multiplier;
	ctmo.ReadTotalTimeoutConstant = 100 * multiplier;
	ctmo.WriteTotalTimeoutMultiplier = 1;
	ctmo.WriteTotalTimeoutConstant = 1;
	if (0 == SetCommTimeouts(hComm, &ctmo))
		return 1;
	return 0;
}

/*@	\brief initialize serial port
 *
 *	\param[uint8_t *] serial   serial port name i.e. "com3"
 *	\return                    1 error,
 *                                 0 OK
 */

int updi_serial_init(char *serial)
{
	char comname[256];

	if (strlen(serial) > 200)
		return 1;

	strcpy(comname, "\\\\.\\");
	strcat(comname, serial);

	hComm = CreateFile(comname, GENERIC_READ | GENERIC_WRITE,	//Read/Write
			   0,	// No Sharing
			   NULL,	// No Security
			   OPEN_EXISTING,	// Open existing port only
			   0,	// Non Overlapped I/O
			   NULL);	// Null for Comm Devices

	if (hComm == INVALID_HANDLE_VALUE) {
		printf("Error in opening serial port %s\n", serial);
		return 1;
	}

	memset(&ctmo, 0, sizeof(COMMTIMEOUTS));
	memset(&dcbSerialParams, 0, sizeof(dcbSerialParams));
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	if (!GetCommState(hComm, &dcbSerialParams))
		return 1;
	dcbSerialParams.BaudRate = CBR_115200;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = TWOSTOPBITS;
	dcbSerialParams.Parity = EVENPARITY;

	dcbSerialParams.fBinary = 1;
	dcbSerialParams.fDtrControl = DTR_CONTROL_DISABLE;
	dcbSerialParams.fRtsControl = RTS_CONTROL_DISABLE;

	if (!SetCommState(hComm, &dcbSerialParams))
		return 1;
	if (updi_set_speed(115200))
		return 1;
	printf("serial port OK\n");
	return 0;
}

void updi_serial_close()
{
	if (hComm != INVALID_HANDLE_VALUE)
		CloseHandle(hComm);
}

/*@	\brief initialie UPDI (break)
 *
 *	\return                    1 error,
 *                                 0 OK
 */

int updi_send_break()
{
	char b[] = { 0, 0 };
	DWORD dwNoOfBytesWritten = 0;

	dcbSerialParams.BaudRate = CBR_300;
	if (0 == SetCommState(hComm, &dcbSerialParams))
		return 1;

	WriteFile(hComm, b, 2, &dwNoOfBytesWritten, NULL);

	// check return code from WriteFile ? or this is enough ?
	if (2 != dwNoOfBytesWritten)
		return 1;

	Sleep(80);
	PurgeComm(hComm, PURGE_RXCLEAR);
	if (updi_set_speed(updi_speed))
		return 1;

	Sleep(1);
	return 0;
}

/*@	\brief send and receive from com port
 *
 *	\param[int] timeout comm timeout in uS
 *	\return 0 - error, positive value - number of received chars
 */

int updi_transaction(uint8_t * send, uint16_t s_len, uint8_t * receive, uint16_t r_len, int timeout)
{
	DWORD dwNoOfBytesWritten = 0;
	DWORD dwBytesRead = 0;

	// win uses miliseconds here
	timeout /= 1000;

	ctmo.ReadIntervalTimeout = timeout;
	ctmo.ReadTotalTimeoutMultiplier = timeout;
	ctmo.ReadTotalTimeoutConstant = timeout;

	if (!SetCommTimeouts(hComm, &ctmo))
		return 0;

	if (!WriteFile(hComm, send, s_len, &dwNoOfBytesWritten, NULL))
		return 0;
	if (dwNoOfBytesWritten != s_len)
		return 0;
	ReadFile(hComm, receive, r_len, &dwBytesRead, NULL);
	return dwBytesRead;
}
