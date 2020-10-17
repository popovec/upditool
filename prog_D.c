/*
    prog_D.c

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


 AVR D series specific code - tested on AVR128DA32

 128 KB AVR128DA28 AVR128DA32 AVR128DA48 AVR128DA64

 may work on MCU with smaller memories:
 64 KB  AVR64DA28 AVR64DA32  AVR64DA48   AVR64DA64
 32 KB  AVR32DA28 AVR32DA32  AVR32DA48


 may work on DB series..

*/
#include <stdio.h>
#include <unistd.h>
#include "UPDI_cmd.h"
#include "prog.h"
#include "prog_D.h"

int
write_UROW_D (struct prog *prog)
{
  int ret;

// based on Microchip AVR128DA28/32/48/64 Preliminary Data Sheet, chapter 35.3.7.3 User Row Programming

// step 1,2
  ret = UPDI_cmd_KEY_USERROW ();
  if (ret != 0)
    return ret;
// step 3,4
  ret = UPDI_do_SYSTEM_RESET ();
  if (ret != 0)
    {
      printf ("app_UROW  fail %02x\n", ret);
      return ret;
    }
// step 5,6
  ret = app_wait_UROWPROG ();
  if (ret)
    {
      printf ("UROW PROG NOT RUNNING ?\n");
      return ret;
    }
// userrow programming - if the user has not supplied
// whole userrow, still write all bytes of userrow
// (this operation always does copy 32 bytes from RAM to
// userrow)
  prog->data_size = prog->u->mem->size;
// 7
  ret = write_data_block (prog);
  if (ret < 0)
    {
      printf ("Unable to write urow1\n");
      return ret;
    }
// 8  ASI System Control A (UPDI.ASI_SYS_CTRLA
  ret = UPDI_cmd_STCS (10, 2);
  if (ret < 0)
    {
      printf ("Unable to end urow write\n");
      return ret;
    }
//9,10
  ret = app_wait_UROWPROG_END ();
  if (ret)
    {
      printf ("Unable to end urow write2\n");
      return ret;
    }
//11:
  ret = UPDI_cmd_STCS (7, 0x20);
  if (ret)
    {
      printf ("Unable to end urow write3\n");
      return ret;
    }
//12,13
  ret = UPDI_do_SYSTEM_RESET ();
  if (ret != 0)
    {
      printf ("Unable to end urow (final reset)\n");
      return ret;
    }
  printf ("userrow programming OK\n");

// reenable NVM to allow read userrow back (for verify) - only on unlocked device
  if (prog->locked == 0)
    if ((ret = UPDI_NVM_enable ()) < 0)
      {
	printf ("NVM activation failed\n");
	return ret;
      }
  return 0;
}

// 0 ok
// any other value error 
int
chip_erase_D ()
{
  int ret;
// AVR128 doc, 35.3.7.1 Chip Erase
//
// step 1,2 send KEY and wait for CHIPERASE (bit 3) in UPDI.ASI_KEY_STATUS (reg 7) is active
  ret = UPDI_cmd_KEY_CHIP_ERASE ();
  if (ret != 0)
    return ret;
// step 3,4 do system reset
  ret = UPDI_do_SYSTEM_RESET ();
  if (ret != 0)
    {
      fprintf (stderr, "CHIP_ERASE, reset fail %02x\n", ret);
      return ret;
    }
// step 5,6: wait for LOCKSTATUS (bit 0) in UPDI.ASI_SYS_STATUS (reg 11)
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
// step 7,8 check ERASE_FAILED (bit 6) in UPDI.ASI_SYS_STATUS (reg 11)
  ret = UPDI_cmd_LDCS (C_UPDIreg_ASI_SYS_STATUS);
  if (ret < 0)
    return ret;
  if (ret & 0x40)
    {
      fprintf (stderr, "Chip erase failed (ERASE_FAILED bit set)\n");
      return 1;
    }
  return 0;
}


// AVR128DA EEPROM write algo
// can be used for fuses/lock too 
int
write_NVM_EEPROM_D (struct prog *prog)
{
  int ret, i;
  uint8_t c = 0;
  uint32_t pages = prog->u->mem->size / prog->u->mem->psize;

// prog->r current data in EEPROM
// prot->v new data

  printf ("'%s' write...\n", prog->u->mem->name);
  if (0 != (ret = wait_NVM_IDLE (prog)))
    return ret;

  if (0 != (ret = cmd_ST24_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;

// Always start with NOCMD
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;

// reset address, mode is prt++!
  if (0 != (ret = cmd_ST24_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;

// TODO erase only neccesary bytes

  c = 0x13;			// eeprom erase/write
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;

  for (i = 0; i < pages; i++)
    {
      prog->page = i;
      progressbar (i, pages);

// if MCU is locked, rewrite whole memory
// if not, write only changes
      if (prog->locked == 1)
	ret = write_data_block (prog);
      else
	ret = write_data_block_i (prog);
      if (ret)
	{
	  printf ("\n'%s' write failed (%d)\n", prog->u->mem->name, ret);
	  return ret;
	}
    }
  progressbar (pages, pages);
  printf ("\n'%s' write OK\n", prog->u->mem->name);

  if (0 != (ret = wait_NVM_IDLE (prog)))
    return ret;

  if (0 != (ret = cmd_ST24_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;
  c = 0;
  return UPDI_cmd_ST_BYTE (&c, 1);
}


int
write_flash_D (struct prog *prog)
{
  int ret, i;
  uint8_t c = 0;
  uint32_t pages;

// AVR128DA .. 35.3.7.2 NVM Programming ... points 1,2,3,4,5,6,7,  in UPDI_NVM_enable ()

  if (0 != (ret = UPDI_NVM_enable ()))
    {
      fprintf (stderr, "Waitning for NVM prog failed (%02x)\n", ret);
      return ret;
    }
// TODO do page erase if needed
// for now only warn user ..
  if (prog->chip_erased != 1)
    if (prog->disable_autoerase == 0)
      printf
	("use -e switch with flash programming, page erase is not supported for now\n");

  pages = (prog->data_size + prog->u->mem->psize - 1) / prog->u->mem->psize;

  printf ("Writing FLASH.. (%d pages)\n", pages);
  if (0 != (ret = wait_NVM_IDLE (prog)))
    {
      printf ("NVM not idle\n");
      return ret;
    }
  if (0 != (ret = cmd_ST24_to_PTR (0x1000)))	// nvmctrl.ctrla
    {
      printf ("unable to send NOCMD\n");
      return ret;
    }
// Always start with NOCMD
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;

// reset address, mode is prt++!
  if (0 != (ret = cmd_ST24_to_PTR (0x1000)))	// nvmctrl.ctrla
    return ret;

  c = 2;			// flash write
  if (0 != (ret = UPDI_cmd_ST_BYTE (&c, 1)))
    return ret;
// write data
// 8
  for (i = 0; i < pages; i++)
    {
      prog->page = i;
      progressbar (i, pages);
      if (0 != (ret = write_word_data_block (prog)))
	{
	  printf ("\n");	//progressbar end
	  printf ("Write data to FLASH error (%02x)\n", ret);
	  return ret;
	}
    }
  progressbar (pages, pages);
  printf ("\n");		//progressbar end
  if (0 != (ret = wait_NVM_IDLE (prog)))
    {
      printf ("Timeout, NVM not idle\n");
      return ret;
    }
  if (0 != (ret = cmd_ST24_to_PTR (0x1000)))	// nvmctrl.ctrla
    {
      printf ("unable to send NOCMD\n");
      return ret;
    }
  c = 0;
  return UPDI_cmd_ST_BYTE (&c, 1);

//9,10
  if (0 != (ret = UPDI_do_SYSTEM_RESET ()))
    {
      fprintf (stderr,
	       "Unable to reaset after FLASH programming (final reset)\n");
      return ret;
    }
  return 0;
}
