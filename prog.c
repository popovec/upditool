/*
    prog.c

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

    base code for programming devices..
    
*/
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "UPDI_ll.h"
#include "UPDI_cmd.h"
#include "prog.h"
#include "prog_D.h"
#include "prog_mega0.h"
#include "helpers.h"



int
UPDI_cmd_ST_to_PTR (struct prog *prog, uint32_t addr)
{
  if (prog->mcu->ptr_bytes == 3)
    return cmd_ST24_to_PTR (addr);
  else
    return cmd_ST16_to_PTR (addr);
}

int
app_wait_UROWPROG ()
{
  int ret;
  int t;
  // wait
  for (t = 0; t < 500; t++)
    {
      ret = UPDI_cmd_LDCS (C_UPDIreg_ASI_SYS_STATUS);
      if (ret < 0)
	{
	  printf ("Error\n");
	  return -1;
	}
      if (ret & 4)
	return 0;
    }
  return 1;
}

int
app_wait_UROWPROG_END ()
{
  int ret;
  int t;
  // wait
  for (t = 0; t < 500; t++)
    {
      ret = UPDI_cmd_LDCS (C_UPDIreg_ASI_SYS_STATUS);
      if (ret < 0)
	{
	  printf ("Error\n");
	  return -1;
	}
      if ((ret & 4) == 0)
	return 0;
    }
  return 1;
}

// wait for LOCKSTATUS (bit 0) in UPDI.ASI_SYS_STATUS (reg 11)
// test for 0 - chip is erased (unlock)
// this is same for mega0 and D series..
int
app_wait_CH_ERASE ()
{
  int ret;
  int t;

  for (t = 0; t < 2000; t++)
    {
      ret = UPDI_cmd_LDCS (C_UPDIreg_ASI_SYS_STATUS);
      if (ret < 0)
	{
	  printf ("Error\n");
	  return -1;
	}
      if ((ret & 1) == 0)	// LOCKSTATUS bit
	return 0;
    }
  return 1;
}


static int
updi_init (struct prog *prog)
{
  int ret;
  uint8_t sib[20];
  int timeout;

  for (timeout = 0;; timeout++)
    {
// always recover from error first
      updi_send_break ();
// ignore errors here...      
/// Disable Collision and Contention Detection
      UPDI_cmd_STCS (3, 8);	// UPDI.CTRLB - set  CCDETDIS
// enable inter byte delay .. and request GT = 128 an disable tiemout
      UPDI_cmd_STCS (2, 0x80);

// request GT = 2, no inte byte delay (not safe)
//      UPDI_cmd_STCS (2, 0x06);
/*
UPDI mega0
Value Description
0x0   Reserved
0x1   16 MHz UPDI clock
0x2   8 MHz UPDI clock
0x3   4 MHz UPDI clock (Default Setting)

UPDI AVR
 Value Description
 0x0   32 MHz UPDI clock
 0x1   16 MHz UPDI clock
 0x2   8 MHz UPDI clock
 0x3   4 MHz UPDI clock

*/

// switch UPDI speed and set serial speed
      if (prog->serial_speed < 75000)
	return -1;
      else if (prog->serial_speed < 190000)
	UPDI_cmd_STCS (9, 3);
      else if (prog->serial_speed < 375000)
	UPDI_cmd_STCS (9, 2);
      else if (prog->serial_speed < 900000)
	UPDI_cmd_STCS (9, 1);
//      else if (prog->serial_speed < 1600000)
      //UPDI_cmd_STCS (9, 0);
      else
	return -1;

      if (0 != (ret = updi_set_speed (prog->serial_speed)))
	{
	  fprintf (stderr, "Unable to switch serial port speed\n");
	  return ret;
	}

// read STATUSB.PESIG (this clear error bits)
      if (UPDI_cmd_LDCS (1) >= 0)
	break;

      if (timeout > 5)
	{
	  fprintf (stderr, "Unable to initialize UPDI link\n");
	  return 1;
	}
      printf ("reinitializing\n");
      sleep (0.2);
    }
// check UPDI version
  if ((ret = UPDI_cmd_LDCS (0)) < 0)
    return ret;
  if (ret != 0x30)
    {
      printf
	("Warning, this software was tested on devices with UPDI version 3, but this device UPDI version is %d\n",
	 ret >> 4);
      sleep (5);
    }
#if 0
  printf ("STATUSA: %02x\n", ret);
  if ((ret = UPDI_cmd_LDCS (2)) < 0)
    goto end;
  printf ("CTRLA: %02x\n", ret);

  if ((ret = UPDI_cmd_LDCS (3)) < 0)
    goto end;
  printf ("CTRLB: %02x\n", ret);

  if ((ret = UPDI_cmd_LDCS (1)) < 0)
    return ret;
  printf ("STATUSB: %02x\n", ret);
#endif
  // SIB..
  if (UPDI_cmd_READ_SIB (sib) < 0)
    {
      fprintf (stderr, "Unable to read SIB\n");
      return ret;
    }
  printf ("Device SIB:\n");
  printf ("\tFamily_ID   : '%c%c%c%c%c%c%c'\n", sib[0], sib[1], sib[2],
	  sib[3], sib[4], sib[5], sib[6]);
  printf ("\tRESERVED    : 0x%02x\n", sib[7]);
  printf ("\tNVM_VERSION : '%c%c%c'\n", sib[8], sib[9], sib[10]);
  printf ("\tOCD_VERSION : '%c%c%c'\n", sib[11], sib[12], sib[13]);
  printf ("\tRESERVED    : 0x%02x\n", sib[14]);
  printf ("\tDBG_OSC_FREQ: 0x%02x", sib[15]);
  printf ("\n");
  if (0 != memcmp (prog->mcu->sib, sib, 7))
    {
      fprintf (stderr,
	       "SIB Family_ID %c%c%c%c%c%c%c does not match device %s\n",
	       sib[0], sib[1], sib[2], sib[3], sib[4], sib[5], sib[6],
	       prog->part_name);
      return 1;
    }
  if (0 == memcmp ("megaAVR", sib, 7))
    {
// megaAVR, device sometime signalize "locked" chip, but after NVM key upload this bit is cleared (device was not locked!)
// to test this case, enable code below ..
#if 0
      if (0 != (ret = UPDI_do_SYSTEM_RESET ()))
	return ret;
#endif
      if ((ret = UPDI_cmd_LDCS (C_UPDIreg_ASI_SYS_STATUS)) < 0)
	return ret;
      if ((ret & 1) == 1)
	{
	  printf
	    ("Seems, this device is locked, but this is megaAVR series, doing workaround\n");
	  if (0 != (ret = UPDI_cmd_KEY_NVM_PROG ()))
	    {
	      fprintf (stderr, "[%s] NVM_enable, key fail %02x\n",
		       __FUNCTION__, ret);
	      return ret;
	    }
	  if ((ret = UPDI_cmd_LDCS (C_UPDIreg_ASI_SYS_STATUS)) < 0)
	    return ret;
	  if (0 != (ret = UPDI_do_SYSTEM_RESET ()))
	    {
	      fprintf (stderr, "[%s] NVM_enable, reset fail %02x\n",
		       __FUNCTION__, ret);
	      return ret;
	    }
	  if (0 != (ret = UPDI_wait_NVMPROG ()))
	    {
	      printf
		("megaAVR series woraround: NVM is not active, seems device is really locked\n");
	      if ((ret = UPDI_cmd_LDCS (C_UPDIreg_ASI_SYS_STATUS)) < 0)
		return ret;
	      printf ("ASI SYS STATUS: %02x\n", ret);
	      return 0;
	    }
	  printf
	    ("megaAVR series woraround: NVM is active, device is unlocked\n");
	}
    }
  return 0;
}

// -1 error
// 0 OK
// 1 timeout
int
wait_NVM_IDLE (struct prog *prog)
{
  int ret;
  uint8_t c;
  int timeout;

  for (timeout = 0; timeout < 1000; timeout++)
    {
// reset address (PTR++ in UPDI_cmd_LD_BYTE)
      ret = UPDI_cmd_ST_to_PTR (prog, 0x1002);	// nvmctrl.status
      if (ret)
	return ret;
      ret = UPDI_cmd_LD_BYTE (&c, 1);
      if (ret)
	return 0;
      if (c & 3)
	continue;
      // clear error bits if set
      if (c)
	{
	  c = 0;
	  return UPDI_cmd_ST_BYTE (&c, 1);
	}
      return 0;
    }
  return 1;
}

void
progressbar (int i, int max)
{

  int j;
  printf ("\r");
  i = i * 50;
  i /= max;

  for (j = 0; j < 50; j++)
    {
      if (j < i)
	printf ("#");
      else
	printf (".");

      fflush (stdout);
    }
  printf ("  [% 3d%% ]", i * 2);
  fflush (stdout);
}

// 0 OK, 1 fail
int
read_data_block (struct prog *prog)
{
  int ret;
  uint32_t addr = prog->u->mem->start;
  uint32_t size = prog->data_size;
  uint8_t *data = prog->r;

  while (size)
    {
      if (0 != (ret = UPDI_cmd_ST_to_PTR (prog, addr)))
	return ret;
      if (size > 256)
	{
	  ret = UPDI_cmd_LD_BYTE (data, 256);
	  if (ret)
	    return ret;
	  data += 256;
	  size -= 256;
	  addr += 256;
	}
      else
	{
	  ret = UPDI_cmd_LD_BYTE (data, size);
	  if (ret)
	    return ret;
	  size = 0;
	}
      progressbar (prog->data_size - size, prog->data_size);
    }
  printf ("\n");
  return 0;
}


// prog->r actual content
// prog->v new contetnt
// change only neccesary bytes
int
write_data_block_i (struct prog *prog)
{
  int ret;
//  uint32_t size = prog->u->mem->size;
  uint32_t size = prog->data_size;
  uint32_t d_count = 0;
  uint32_t mcu_addr = prog->u->mem->wstart;
  uint8_t *data = prog->v;
  uint8_t *old_data = prog->r;

  if (size > prog->u->mem->psize)
    {
      size = prog->u->mem->psize;
      mcu_addr += prog->page * prog->u->mem->psize;
      data += prog->page * prog->u->mem->psize;
      old_data += prog->page * prog->u->mem->psize;
    }

  for (; size;)
    {
      // skip for same values..
      if (*data == *old_data)
	{
	  d_count++;
	  data++;
	  old_data++;
	  mcu_addr++;
	  size--;
	  continue;
	}
//      printf ("skippng %d bytes\n", d_count);
      // data differ...  count how many bytes are different
      d_count = 0;
      while (d_count < size)
	if (data[d_count] != old_data[d_count])
	  d_count++;
	else
	  break;

//      printf ("programming %d bytes\n", d_count);

      ret = UPDI_cmd_ST_to_PTR (prog, mcu_addr);
      if (ret)
	return ret;
      while (d_count > 256)
	{
	  ret = UPDI_cmd_ST_BYTE (data, 256);
	  if (ret)
	    return ret;
	  data += 256;
	  old_data += 256;
	  mcu_addr += 256;
	  d_count -= 256;
	  size -= 256;
	}
      if (d_count)
	{
	  ret = UPDI_cmd_ST_BYTE (data, d_count);
	  if (ret)
	    return ret;
	  data += d_count;
	  old_data += d_count;
	  mcu_addr += d_count;
	  size -= d_count;
	  d_count = 0;
	}
//      progressbar (prog->data_size - size, prog->data_size);
    }
//  printf ("\n");
  return 0;
}

int
write_data_block (struct prog *prog)
{
  int ret;
//  uint32_t size = prog->u->mem->size;
  uint32_t size = prog->data_size;
  uint32_t mcu_addr = prog->u->mem->wstart;
  uint8_t *data = prog->v;

  if (size > prog->u->mem->psize)
    {
      size = prog->u->mem->psize;
      mcu_addr += prog->page * prog->u->mem->psize;
      data += prog->page * prog->u->mem->psize;
    }

  if (0 != (ret = UPDI_cmd_ST_to_PTR (prog, mcu_addr)))
    return ret;

  while (size)
    {
      if (size > 128)
	{
	  ret = UPDI_cmd_ST_BYTE_RSD (data, 128);
	  if (ret)
	    return ret;
	  data += 128;
	  size -= 128;
	}
      else
	{
	  ret = UPDI_cmd_ST_BYTE_RSD (data, size);
	  if (ret)
	    return ret;
	  size = 0;
	}
//      progressbar (prog->data_size - size, prog->data_size);
    }
//  printf ("\n");
  return 0;
}



int
write_word_data_block (struct prog *prog)
{
  int ret;
//  uint32_t size = prog->u->mem->size;
  uint32_t size = prog->data_size;
  uint32_t mcu_addr = prog->u->mem->wstart;
  uint8_t *data = prog->v;
  uint32_t psize = prog->u->mem->psize;

  if (size > prog->u->mem->psize)
    {
      size = prog->u->mem->psize;
      mcu_addr += prog->page * prog->u->mem->psize;
      data += prog->page * prog->u->mem->psize;
    }


  if (0 != (ret = UPDI_cmd_ST_to_PTR (prog, mcu_addr)))
    return ret;

  while (size)
    {
      if (size > psize)
	{
	  ret = UPDI_cmd_ST_WORD_RSD (data, psize / 2);
	  if (ret)
	    return ret;
	  data += psize;
	  size -= psize;
	}
      else
	{
	  ret = UPDI_cmd_ST_WORD_RSD (data, size / 2);
	  if (ret)
	    return ret;
	  size = 0;
	}
//      progressbar (prog->data_size - size, prog->data_size);
    }
//  printf ("\n");
  return 0;
}



static int
prog_write_mcu (struct prog *prog)
{
  int ret;

  if (0 == memcmp (prog->r, prog->v, prog->data_size))
    {
      printf
	("skipping programming '%s', because already same content\n",
	 prog->u->mem->name);
      return 0;
    }
  switch (prog->u->mem->write_algo)
    {
    case C_WRITE_ALGO_FLASH_D:
      ret = write_flash_D (prog);
      break;

    case C_WRITE_ALGO_USERROW_D:
      ret = write_UROW_D (prog);
      break;

    case C_WRITE_ALGO_NVM_EEPROM_D:
      ret = write_NVM_EEPROM_D (prog);
      break;

    case C_WRITE_ALGO_FUSEmega0:
      ret = write_fuse_mega0 (prog);
      break;

    case C_WRITE_ALGO_NVMmega0:
      ret = write_NVM_mega0 (prog);
      break;

    default:
      fprintf (stderr,
	       "Algorhithm for write to %s is not implemented for now :-(\n",
	       prog->u->mem->name);
      ret = 1;
      break;
    }

  if (ret)
    fprintf (stderr, "Unable to write do device %s memory %s\n",
	     prog->mcu->name, prog->u->mem->name);

  return ret;
}

static int
prog_read_mcu (struct prog *prog)
{
  int ret;
// TODO
//  if (prog->u->mem->nvm == 1)
  // TEST if NVM is enabled
  ret = read_data_block (prog);

  if (ret)
    printf ("Unable to read device memory '%s'\n", prog->u->name);
  return ret;
}

static int
check_signature (struct prog *prog)
{
  // create fictive U operation to get signature
  struct U u_sig;

  prog->u = &u_sig;
// test signature:
  prog->u->mem = search_memory_by_name (prog->mcu, "signature");
  if (NULL == prog->u->mem)
    {
      fprintf (stderr, "Unable to find device signature specification\n");
      return 1;
    }
  prog->data_size = prog->u->mem->size;
  // read and test signature
  if (read_data_block (prog))
    {
      printf ("Unable to read device signature\n");
      return 1;
    }

  printf ("Signature bytes %02x %02x %02x\n", prog->r[0],
	  prog->r[1], prog->r[2]);
  if (0 != memcmp (prog->r, prog->mcu->signature, 3))
    {
      printf ("Device signature does not match specified device\n");
      if (prog->skip_signature_check == 1)
	printf ("Continuing because -F switch was set\n");
      else
	return 1;
    }
  return 0;
}

static int
run_U_operation (struct prog *prog)
{
  struct U *u;
  int ret;

  // proceed all memory operations 
  for (u = prog->U_first; u != NULL; u = u->next)
    {
      prog->u = u;
      prog->data_size = u->mem->size;

      printf ("running OP: memory '%s', operation '%c'\n", u->name, u->op);

      // for 'w' and 'v' operation we need to read input file
      if (u->op == 'w' || u->op == 'v')
	{
	  if ((ret =
	       file_read (u->filename, u->fmt, prog->v, u->mem->size)) < 1)
	    {
	      printf ("Unable to read file '%s' %d\n", u->filename, ret);
	      return 1;
	    }
	  prog->data_size = ret;
	}

      // clear buffer
      memset (prog->r, 0xff, u->mem->size);
      if (prog->locked == 0)
	{
	  // always read specified memory
	  // read or verify operation, we need data from MCU
	  if (prog->chip_erased
	      && (prog->u->mem->not_affected_by_chip_erase == 0))
	    printf ("Skipping read operation, chip erase was performed\n");
	  else
	    {
	      printf ("reading '%s'\n", u->mem->name);
	      if (0 != (ret = prog_read_mcu (prog)))
		return ret;;
	      printf ("MCU read OK\n");
	    }
	  if (u->op == 'r' || u->op == 'v')
	    {
	      // data from MCU to file
	      if (u->op == 'r')
		{
		  ret =
		    file_write (u->filename, u->fmt, prog->r,
				prog->u->mem->size);
		  if (ret != u->mem->size)
		    printf
		      ("Unable to write file '%s', continuing %d %d\n",
		       u->filename, ret, prog->u->mem->size);
		}
	      // verify data from MCU and data from file
	      if (u->op == 'v')
		{
		  if (0 == memcmp (prog->r, prog->v, prog->data_size))
		    {
		      printf ("Verify OK\n");
		    }
		  else
		    {
		      printf ("Verify FAILED\n");
		      return 1;
		    }
		}
	    }
	}
      else
	{
	  if (u->op != 'w')
	    {
	      fprintf (stderr,
		       "Device is locked, but requested operation is not write\n");
	      return 1;
	    }
	}

      // write to MCU   
      if (u->op == 'w')
	{
	  if (prog->data_size < prog->u->mem->size)
	    memcpy (prog->v + prog->data_size,
		    prog->r + prog->data_size,
		    prog->u->mem->size - prog->data_size);


	  if (0 != (ret = prog_write_mcu (prog)))
	    return ret;

	  if (prog->skip_verify == 1)
	    continue;

	  if (prog->locked == 0)
	    {
	      // reread (for verification)
	      printf ("Reading MCU memory '%s' to verify write operation\n",
		      u->mem->name);
	      if (0 != (ret = prog_read_mcu (prog)))
		return ret;
	      if (0 == memcmp (prog->r, prog->v, prog->data_size))
		{
		  printf ("Verify OK\n");
		}
	      else
		{
		  printf ("Verify FAILED\n");
		  return 1;
		}
	    }
	  else
	    {
	      printf ("MCU is locked, skipping verification\n");
	    }
	}
    }
  return 0;
}

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int
run_prog (struct prog *prog)
{
  int ret;

	if (0 != updi_serial_init (prog->serial_port))
		return 1;


  for (;;)
    {
// initialize UPDI, check version, load and check SIB
      if (0 != (ret = updi_init (prog)))
	return ret;
      printf ("Init ok\n");

/* 
AVR128DA - if device is locked, there is no way to read device signature
if chip erase is specified, 1st do chip erase then normal read/write 
User row programming is not affected if NVM is not activated.
*/

// check device status - is locked ? 
      if ((ret = UPDI_cmd_LDCS (C_UPDIreg_ASI_SYS_STATUS)) < 0)
	goto end;
      if (ret & 1)
	{
	  prog->locked = 1;
	  if (prog->do_chip_erase == 0)
	    printf
	      ("Warning, device is locked and no chip erase is specified, only userrow programming is available\n");
	}
      else
	{
	  printf ("Device is not locked\n");
// register file and RAM can be accessed, but no FLASH and EEPROM
// we need to activate NVM (to check MCU signature)

	  if ((ret = UPDI_NVM_enable ()) < 0)
	    {
	      printf ("NVM activation failed\n");
	      goto end;
	    }

	  if (0 != (ret = check_signature (prog)))
	    goto end;
	}

      if (prog->do_chip_erase == 1)
	{
	  switch (prog->mcu->chip_erase_algo)
	    {
	    case C_ALGO_CHIP_ERASE_D:
	      ret = chip_erase_D ();
	      break;
	    case C_ALGO_CHIP_ERASE_mega0:
	      ret = chip_erase_mega0 ();
	      break;
	    default:
	      fprintf (stderr,
		       "Unable to found chip erase algo for this mcu\n");
	      ret = 1;
	    }
	  if (ret)
	    goto end;
	  prog->do_chip_erase = 0;
	  prog->chip_erased = 1;
	  printf ("Chip erase OK, reinitializing UPDI\n");
	  UPDI_cmd_STCS (3, 4);
	}
      else
	break;
    }
  printf ("---------------------------------------\n");
  ret = run_U_operation (prog);

end:
  // ignore return code, if this fail, UPDI is disabled and reset is activated ..
  // ret value below 0 is fatal, skip UPDI_do_SYSTEM_RESET () - device is not responding...
  if (ret >= 0)
    UPDI_do_SYSTEM_RESET ();
// disable UPDI (this clear all keys loaded and system reset is performed)
  printf ("disabling UPDI\n");
  UPDI_cmd_STCS (3, 4);
  updi_serial_close ();

  return ret;
}
