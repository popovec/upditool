/*
    UPDI_cmd.c

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

    UPDI high level commands
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "UPDI_cmd.h"
#include "UPDI_ll.h"

#define T_NORMAL 5000

// -1 error
// 0..255 return value
int UPDI_cmd_LDCS(uint8_t addr)
{
	uint8_t cmd[2];
	uint8_t reply[5];
	int ret;
	int i;

	if (addr > 13) {
		fprintf(stderr, "Internal error\n");
		exit(1);
	}
	cmd[0] = 0x55;
	cmd[1] = 0x80 | (addr & 0xf);

	ret = updi_transaction(cmd, 2, reply, 4, T_NORMAL);
	if (ret != 3) {
		fprintf(stderr,
			"[%s] updi transaction failed, register %d, returned %d chars, expected 3 chars\n[",
			__FUNCTION__, addr, ret);
		for (i = 0; i < ret; i++) {
			fprintf(stderr, "%02x ", reply[i]);
		}
		fprintf(stderr, "]\n");
		return -1;
	}
	return reply[2];
}

// -1 error
// 0 OK
int UPDI_cmd_STCS(uint8_t addr, uint8_t data)
{
	uint8_t cmd[3];
	uint8_t reply[5];
	int ret;
	int i;

	if (addr > 13) {
		fprintf(stderr, "Internal error\n");
		exit(1);
	}

	cmd[0] = 0x55;
	cmd[1] = 0xc0 | (addr & 0xf);
	cmd[2] = data;

	ret = updi_transaction(cmd, 3, reply, 4, T_NORMAL);
	if (ret != 3) {
		fprintf(stderr,
			"[%s] updi transaction failed, returned %d chars, expected 3 chars\n[",
			__FUNCTION__, ret);
		for (i = 0; i < ret; i++) {
			fprintf(stderr, "%02x ", reply[i]);
		}
		fprintf(stderr, "]\n");
		return -1;
	}
	// after STCS - do reread register, there is no ACK and we need to check if
	// device received this command...

	// exceptions:
	// register 8 (ASI Reset Request)
	if (addr == 8)
		return 0;
	// register 3, UPDI reset
	if ((addr == 3) && (data & 4))
		return 0;
	// asi key status, there is write into this reg after userrow programming.. ignore reread here
	if (addr == 7)
		return 0;
	// clock ..
	if (addr == 9)
		return 0;

	ret = UPDI_cmd_LDCS(addr);
	if (ret < 0)
		return ret;
	if (ret != data) {
		fprintf(stderr, "[%s] Data=%02x reread=%02x\n", __FUNCTION__, data, ret);
		return -1;
	}
	return 0;
}
/* *INDENT-OFF* */


int
cmd_ST24_to_PTR (uint32_t addr)
{
  uint8_t cmd[5];
  uint8_t r[7];
  int ret;
  cmd[0] = 0x55;
  cmd[1] = 0x6a;		// write 3 bytes to PTR
  cmd[4] = (addr >> 16) & 0xff;
  cmd[3] = (addr >> 8) & 0xff;
  cmd[2] = addr & 0xff;
  ret = updi_transaction (cmd, 5, r, 7, T_NORMAL);
  if (ret != 6)
    {
      printf ("no ack\n");
      return 1;
    }
  if (r[5] != 0x40)
    {
      printf ("Wrong ack %0x\n", r[5]);
      return 1;
    }
  return 0;
}

int
cmd_ST16_to_PTR (uint32_t addr)
{
  uint8_t cmd[4];
  uint8_t r[6];
  int ret;
  cmd[0] = 0x55;
  cmd[1] = 0x69;		// write 2 bytes to PTR
  cmd[3] = (addr >> 8) & 0xff;
  cmd[2] = addr & 0xff;
  ret = updi_transaction (cmd, 4, r, 6, T_NORMAL);
  if (ret != 5)
    {
      printf ("no ack\n");
      return 1;
    }
  if (r[4] != 0x40)
    {
      printf ("Wrong ack %0x\n", r[4]);
      return 1;
    }
  return 0;
}


// -1 error
// 0 OK
//
int
UPDI_cmd_LD_BYTE (uint8_t * buffer, int count)
{
  uint8_t cmd[3];
  uint8_t r[259];
  int ret;

  if (count > 256 || count < 1)
    {
      fprintf (stderr, "[%s] Internal error, count below 0  or over 256 %d\n",
	       __FUNCTION__, count);
      exit (1);
    }
  cmd[0] = 0x55;		// SYNC

  // SET REPEAT if needed
  if (count > 1)
    {
      cmd[1] = 0xa0;		// REPEAT (BYTE)
      cmd[2] = count - 1;
      ret = updi_transaction (cmd, 3, r, 5, T_NORMAL);
      if (ret != 3)
	{
	  printf ("[%s], set REPEAT - someting is wrong\n", __FUNCTION__);
	  return -1;
	}
    }
  cmd[1] = 0x24;		// LD from *(ptr++)
  ret = updi_transaction (cmd, 2, r, 2 + count + 2, T_NORMAL);
  if (ret != 2 + count)
    {
      printf ("[%s] someting is wrong (received %d but assumed %d)\n",
	      __FUNCTION__, ret, 2 + count);
      return -1;
    }
  memcpy (buffer, r + 2, count);
  return 0;
}



// ***********************************************************************
/*

Store byte/bytes  or word/words into data space
If bytes/words are stored, then UPDI REPEAT instruction
is used.

Can be used to write to RAM IO filoe, EEPROM/FLASH (before
FLASH or EEPROM write enable NVM )

if   'b' is set, fo word write
if 'rsd' is set, disable 'ack' fro MCU to speed up transfer

UPDI ptr  must be set before this call.
*/

static int
UPDI_cmd_ST (uint8_t * buffer, int count, int b, int rsd)
{
  uint8_t cmd[4];
  uint8_t r[6];
  int ret;
//  int repeat;

  // TEST size
  if (count > 256 || count < 1)
    {
      fprintf (stderr, "[%s] Internal error, count below 0  or over 256 %d\n",
	       __FUNCTION__, count);
      exit (1);
    }
  if (b < 0 || b > 1)
    {
      fprintf (stderr, "[%s] Internal error, b not 0/1%d\n", __FUNCTION__, b);
      exit (1);
    }
  cmd[0] = 0x55;		//SYNC
  // one BYTE/WORD ?

  if (count == 1)
    {
      // do not set repeat, do not disable Remote Signature

      // write 1st BYTE/WORD
      cmd[1] = 0x64 | b;	// ST *(PTR++), BYTE or WORD
      cmd[2] = *(buffer++);
      if (b)
	cmd[3] = *(buffer++);
      ret = updi_transaction (cmd, 3 + b, r, 5 + b, T_NORMAL);
      if (ret != 4 + b)
	{
	  fprintf (stderr, "[%s] someting is wrong\n", __FUNCTION__);
	  return -1;
	}
      if (r[3 + b] != 0x40)
	{
	  fprintf (stderr, "[%s] Wrong ack %0x\n", __FUNCTION__, r[3 + b]);
	  return -1;
	}
      return 0;
    }

  // SET REPEAT
  cmd[1] = 0xa0;		// REPEAT (BYTE)
  cmd[2] = count - 1;
  ret = updi_transaction (cmd, 3, r, 5, T_NORMAL);
  if (ret != 3)
    {
      fprintf (stderr, "[%s], set REPEAT - someting is wrong\n",
	       __FUNCTION__);
      return -1;
    }
  if (rsd)
    {
      uint8_t cmd_full[514];	// 0x55,0x65 ST *(ptr++),WORD
      uint8_t resp_full[514];

      // disable remote signature
      // do not use UPDI_cmd_STCS here, becahuse this fails on ACK..
      cmd_full[0] = 0x55;
      cmd_full[1] = 0xc2;
      cmd_full[2] = 0x88;
      ret = updi_transaction (cmd_full, 3, resp_full, 4, T_NORMAL);
      if (ret != 3)
	{
	  printf ("unable to set RSD\n");
	  return -1;
	}
      // write all in one chunk
      cmd_full[0] = 0x55;
      cmd_full[1] = 0x64 | b;	// ST *(PTR++), BYTE or WORD
      memcpy (cmd_full + 2, buffer, count * (1 + b));
      ret =
	updi_transaction (cmd_full, 2 + count * (1 + b), resp_full,
			  3 + count * (1 + b), T_NORMAL);
      if (ret != 2 + count * (1 + b))
	{
	  fprintf (stderr,
		   "[%s] write transaction someting is wrong (ret=%d)\n",
		   __FUNCTION__, ret);
	  return -1;
	}

      // do not use UPDI_cmd_STCS here, because this fails on ACK..
      cmd_full[0] = 0x55;
      cmd_full[1] = 0xc2;
      cmd_full[2] = 0x80;
      ret = updi_transaction (cmd_full, 3, resp_full, 4, T_NORMAL);
      if (ret != 3)
	{
	  printf ("unable to revert RSD\n");
	  return -1;
	}
      cmd_full[1] = 0x81;
      ret = updi_transaction (cmd_full, 2, resp_full, 4, T_NORMAL * 10);
      if (ret != 3)
	{
	  printf ("unable to revert RSD, read from STATUS B failed\n");
	  return -1;
	};
      return 0;
    }
  // write 1st byte
  cmd[1] = 0x64 | b;		// ST *(PTR++), BYTE or WORD
  cmd[2] = *(buffer++);

  if (b)
    cmd[3] = *(buffer++);

  ret = updi_transaction (cmd, 3 + b, r, 5 + b, T_NORMAL);
  if (ret != 4 + b)
    {
      fprintf (stderr, "[%s] someting is wrong\n", __FUNCTION__);
      return -1;
    }
  if (r[3 + b] != 0x40)
    {
      fprintf (stderr, "[%s] Wrong ack %0x\n", __FUNCTION__, r[3 + b]);
      return -1;
    }
  // write rest ..
  while (--count)
    {
      ret = updi_transaction (buffer, 1 + b, r, 3 + b, T_NORMAL);
      buffer++;
      buffer += b;
      if (ret != 2 + b)
	{
	  fprintf (stderr, "[%s] (repeat) someting is wrong\n", __FUNCTION__);
	  return -1;
	}
      if (r[1 + b] != 0x40)
	{
	  fprintf (stderr, "[%s] (repeat) Wrong ack %0x\n", __FUNCTION__,
		   r[1 + b]);
	  return -1;
	}
    }
  return 0;
}

// -1 error
// 0 OK
//
int
UPDI_cmd_ST_BYTE (uint8_t * buffer, int count)
{
  return UPDI_cmd_ST (buffer, count, 0, 0);
}

// -1 error
// 0 OK
//
int
UPDI_cmd_ST_BYTE_RSD (uint8_t * buffer, int count)
{
  return UPDI_cmd_ST (buffer, count, 0, 1);
}

// -1 error
// 0 OK
//
int
UPDI_cmd_ST_WORD_RSD (uint8_t * buffer, int count)
{
  return UPDI_cmd_ST (buffer, count, 1, 1);
}

static int
UPDI_run_RESET ()
{
  int ret;
  int t;

  // apply reset
  ret = UPDI_cmd_STCS (8, 0x59);
  if (ret < 0)
    return ret;
  if (ret != 0)
    return ret;
  // wait
  for (t = 0; t < 500; t++)
    {
      ret = UPDI_cmd_LDCS (11);
      if (ret < 0)
	{
	  fprintf (stderr, "[%s] error in LDCS\n", __FUNCTION__);
	  return -1;
	}
      if (ret & 0x20)
	return 0;
      usleep (10000);
    }
  fprintf (stderr, "[%s] timeout\n", __FUNCTION__);
  return 1;
}

static int
UPDI_stop_RESET ()
{
  int ret;
  int t;

  // release reset
  ret = UPDI_cmd_STCS (8, 0);
  if (ret < 0)
    return ret;
  if (ret != 0)
    return ret;
  // wait
  for (t = 0; t < 500; t++)
    {
      ret = UPDI_cmd_LDCS (11);
      if (ret < 0)
	{
	  fprintf (stderr, "[%s] error in LDCS\n", __FUNCTION__);
	  return -1;
	}
      if ((ret & 0x20) == 0)
	return 0;
      usleep (10000);
    }
  fprintf (stderr, "[%s] timeout\n", __FUNCTION__);
  return 1;
}

// -1 transaction error
// 0 OK
// 1 timeout
int
UPDI_do_SYSTEM_RESET ()
{
  int ret;

  ret = UPDI_run_RESET ();
  if (ret != 0)
    return ret;
  return UPDI_stop_RESET ();
}

// check if key is active (use 'mask' to ASI_KEY_STATUS, reg 7), if not, 
// send key repeat test .. 
// -1 transaction error or key size error
// 0 OK
// 1 timeout
int
UPDI_cmd_KEY (uint8_t * key, uint8_t size, uint8_t mask)
{
  int ret;
  int repeat;

  uint8_t s[34];
  uint8_t r[35];

  if (size > 32)
    return -1;

  s[0] = 0x55;
  s[1] = 0xe0;
  memcpy (s + 2, key, size);

  for (repeat = 0; repeat < 5; repeat++)
    {
      if ((ret = UPDI_cmd_LDCS (7)) < 0)	// C_UPDIreg_ASI_KEY_STATUS
	return ret;
      if (ret & mask)
	return 0;

      ret = updi_transaction (s, 2 + size, r, size + 2 + 1, T_NORMAL);
      if (ret != 2 + size)
	{
	  printf ("[%s] transaction fail\n", __FUNCTION__);
	  return -1;
	}
      if (repeat)
	usleep (100000);
    }
  printf ("[%s] timeout\n", __FUNCTION__);
  return 1;
}


int
UPDI_cmd_KEY_NVM_PROG ()
{
  int ret;
  uint8_t key[] = { 0x20, 0x67, 0x6f, 0x72, 0x50, 0x4d, 0x56, 0x4e };

  if (0 != (ret = UPDI_cmd_KEY (key, 8, 0x10)))
    fprintf (stderr, "Unable to activate NVM_PROG key\n");

  return ret;
}

int
UPDI_cmd_KEY_CHIP_ERASE ()
{
  int ret;
  uint8_t key[] = { 0x65, 0x73, 0x61, 0x72, 0x45, 0x4d, 0x56, 0x4e };

  if (0 != (ret = UPDI_cmd_KEY (key, 8, 0x08)))
    fprintf (stderr, "Unable to activate CHIP ERASE key\n");
  return ret;
}

int
UPDI_cmd_KEY_USERROW ()
{
  int ret;
  uint8_t key[] = { 0x65, 0x74, 0x26, 0x73, 0x55, 0x4d, 0x56, 0x4e };

  if (0 != (ret = UPDI_cmd_KEY (key, 8, 0x20)))
    fprintf (stderr, "Unable to activate USERROW key\n");
  return ret;
}

int
UPDI_cmd_READ_SIB (uint8_t * buffer)
{
  uint8_t cmd[2] = { 0x55, 0xe5 };
  uint8_t r[19];
  int ret;

  printf ("READ_SIB\n");

  ret = updi_transaction (cmd, 2, r, 2 + 16 + 1, T_NORMAL);
  if (ret != 2 + 16)
    {
      printf
	("cmd_READ_SIB someting is wrong, requested bytes %d read bytes %d\n",
	 2 + 16, ret);
      return 1;
    }
  memcpy (buffer, r + 2, 16);
  return 0;
}

// -1 error
// 0 OK
// 1 timeout
int
UPDI_wait_NVMPROG ()
{
  int ret;
  // wait
  int t;
  printf ("Waiting for NVM activation..\n");
  for (t = 0; t < 250; t++)
    {
      ret = UPDI_cmd_LDCS (11);	//C_UPDIreg_ASI_SYS_STATUS)
      if (ret < 0)
	{
	  printf ("Error\n");
	  return -1;
	}
      if (ret & 8)
	{
	  printf ("OK NVM is active\n");
	  return 0;
	}
      usleep (10000);
    }
  printf ("timeout waiting for NVM enable\n");
  return 1;
}

// -1 error
// 0 OK
// 1 timeout
int
UPDI_NVM_enable ()
{
  int ret;

  ret = UPDI_do_SYSTEM_RESET ();
  if (ret != 0)
    {
      fprintf (stderr, "[%s] NVM_enable, reset fail %02x\n", __FUNCTION__,
	       ret);
      return ret;
    }
// AVR128DA    35.3.7.2 NVM Programming, step 2,3
// mega4808..  30.3.7.2 NVM Programming, step 2,3
  if (0 != (ret = UPDI_cmd_KEY_NVM_PROG ()))
    {
      fprintf (stderr, "[%s] NVM_enable, key fail %02x\n", __FUNCTION__, ret);
      return ret;
    }
// AVR128DA    35.3.7.2 NVM Programming, step 4,5
// mega4808..  30.3.7.2 NVM Programming, step 4,5
  if (0 != (ret = UPDI_do_SYSTEM_RESET ()))
    {
      fprintf (stderr, "[%s] NVM_enable, reset fail %02x\n", __FUNCTION__,
	       ret);
      return ret;
    }
// AVR128DA    35.3.7.2 NVM Programming, step 6,7
// mega4808..  30.3.7.2 NVM Programming, step 6,7
  ret = UPDI_wait_NVMPROG ();
  return ret;
}
/* *INDENT-ON* */
