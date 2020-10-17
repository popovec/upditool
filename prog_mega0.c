/*
    prog_mega0.c

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

    ATmega4808/4809 specific code

*/
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "UPDI_cmd.h"
#include "prog.h"
#include "prog_mega0.h"
/*
ATmega4808/4809 doc, chapter 9.3.2.4.7
check if NVM is not busy (flash/eeprom)
load fuse address NVMCTRL.ADDR
load fuse value   NVMCTRL.DATA

fuse write command
  unlock  CPU.CCP	--- not needed because cmd from UPDI
  command NVMCTRL.CTRLA  0x7

wait nvm ..
(reset)

*/


int
write_fuse_mega0 (struct prog *prog)
{
  int ret;

  uint8_t c = 0;

// prog->r current data in MCU
// prot->v new data

  printf ("INITIAL Waiting NVM IDLE\n");
  if (0 != (ret = wait_NVM_IDLE (prog)))
    return ret;

  printf ("INITIAL sending NOCMD\n");
  if (0 != (ret = cmd_ST16_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;

// Always start with NOCMD
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;

  printf ("setting fuse address\n");
  if (0 != (ret = cmd_ST16_to_PTR (0x1008)))	// nvmctrl.ADDR
    return ret;
// address L
  c = prog->u->mem->start & 0xff;
  printf ("address L=%02x\n", c);
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;
// address H
  c = (prog->u->mem->start >> 8) & 0xff;
  printf ("address H=%02x\n", c);
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;

  printf ("setting fuse data\n");
  if (0 != (ret = cmd_ST16_to_PTR (0x1006)))	// nvmctrl.DATA
    return ret;

  c = prog->v[0] & 0xff;
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;
// command
  if (0 != (ret = cmd_ST16_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;
  c = 7;
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;

  printf ("INITIAL Waiting NVM IDLE\n");
  if (0 != (ret = wait_NVM_IDLE (prog)))
    return ret;

  return 0;
}

// mega4808 doc, chapter 9.3.2.3
static int
write_NVM_mega0_page (struct prog *prog)
{
  int ret;
  uint16_t mcu_addr = prog->u->mem->wstart;
  uint8_t c = 0;

  mcu_addr += prog->page * prog->u->mem->psize;

  if (0 != (ret = wait_NVM_IDLE (prog)))
    {
      printf ("NVM not idle\n");
      return ret;
    }

  // clear page buffer
  c = 4;
  if (0 != (ret = cmd_ST16_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;

  if (0 != (ret = wait_NVM_IDLE (prog)))
    {
      printf ("NVM not idle\n");
      return ret;
    }

  if (0 != (ret = cmd_ST16_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;

  // write page buffer
  if (0 != (ret = write_data_block (prog)))
    return ret;

  c = 3;

  // erase page, write page buffer
  if (0 != (ret = cmd_ST16_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;
  usleep (10000);

  if (0 != (ret = wait_NVM_IDLE (prog)))
    {
      printf ("NVM not idle\n");
      return ret;
    }
  c = 0;
  if (0 != (ret = cmd_ST16_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;

  return 0;
}

int
write_NVM_mega0 (struct prog *prog)
{
  uint32_t pages = prog->u->mem->size / prog->u->mem->psize;
  int i;
  int ret;

  pages = (prog->data_size + prog->u->mem->psize - 1) / prog->u->mem->psize;

  printf ("Writing NVM '%s'... (%d pages)\n", prog->u->mem->name, pages);

  for (i = 0; i < pages; i++)
    {
      prog->page = i;
      progressbar (i, pages);

      if (0 != (ret = write_NVM_mega0_page (prog)))
	{
	  printf ("\n");
	  return ret;
	}
    }
  progressbar (pages, pages);
  printf ("\n");
  return 0;
}

// chip erase over NVM mega4808 doc, chapter 9.3.2.4.5 or
//         using UPDI  mega4808 doc, chapter 30.3.7.1

// based on mega4808 doc, chapter 30.3.7.1
// 0 ok
// any other value error
int
chip_erase_mega0 ()
{
  int ret;

  printf ("Chip erase..\n");
  ret = UPDI_cmd_KEY_CHIP_ERASE ();
  if (ret != 0)
    return ret;

  ret = UPDI_do_SYSTEM_RESET ();
  if (ret != 0)
    {
      fprintf (stderr, "CHIP_ERASE, reset fail %02x\n", ret);
      return ret;
    }

  ret = app_wait_CH_ERASE ();
  if (ret)
    {
      fprintf (stderr, "Waiting for LOCKSTATUS clear fail\n");
      return ret;
    }
// There is question:
// If device is unlocked and user start the Chip Erase operation, the LOCKSTATUS bit was zero,
// because device was unlocked. The previous app_wait_CH_ERASE_D() ends immediately?
// TODO check this, for now pause here

  sleep (1);
// D seriess allow us to check error after chip erase in
// UPDI.ASI_SYS_STATUS (reg 11) - ERASE_FAILED (bit 6)
// this bi tis reserved in mega0


  printf ("OK\n");
  return 0;
}
