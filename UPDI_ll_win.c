// windows code (based on https://www.xanthium.in/Serial-Port-Programming-using-Win32-API)
#include<windows.h>
#include<stdio.h>
#include <stdint.h>
#include <math.h>
HANDLE hComm;
DCB dcbSerialParams;
COMMTIMEOUTS ctmo;

int updi_serial_init(char *serial)
{
	char comname[256];
	uint8_t multiplier;

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

	multiplier = (uint8_t) ceil((float)100000 / 115200);
	ctmo.ReadIntervalTimeout = 20 * multiplier;
	ctmo.ReadTotalTimeoutMultiplier = 1 * multiplier;
	ctmo.ReadTotalTimeoutConstant = 100 * multiplier;
	ctmo.WriteTotalTimeoutMultiplier = 1;
	ctmo.WriteTotalTimeoutConstant = 1;
	SetCommTimeouts(hComm, &ctmo);
	printf("serial port OK\n");
	return 0;
}

void updi_serial_close()
{
	if (hComm != INVALID_HANDLE_VALUE)
		CloseHandle(hComm);
}

// fixed serial speed..
int updi_set_speed(int speed)
{
	if (speed != 115200)
		return 1;
	return 0;
}

int updi_send_break()
{
	char b[] = { 0, 0 };
	DWORD dwNoOfBytesWritten = 0;
	printf("speed  (300)\n");

	dcbSerialParams.BaudRate = CBR_300;
	if (0 == SetCommState(hComm, &dcbSerialParams))
		return 1;
	printf("sending break\n");
	WriteFile(hComm, b, 2, &dwNoOfBytesWritten, NULL);

	// check return code from WriteFile ? or this is enough ?
	if (2 != dwNoOfBytesWritten)
		return 1;

	Sleep(80);
	PurgeComm(hComm, PURGE_RXCLEAR);
	dcbSerialParams.BaudRate = CBR_115200;
	if (!SetCommState(hComm, &dcbSerialParams))
		return 1;
	printf("restored state (115200)\n");

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
