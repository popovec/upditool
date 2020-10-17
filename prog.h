/*
    prog.h

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
#ifndef __PROGRAMMER_H__
#define __PROGRAMMER_H__

#define MAX_MEMORY_LEN 0x20000

#define C_UPDIreg_ASI_KEY_STATUS  7
#define C_UPDIreg_ASI_SYS_STATUS  11

#define C_WRITE_ALGO_NONE 0
#define C_WRITE_ALGO_NVM_EEPROM_D 10
#define C_WRITE_ALGO_USERROW_D    20
#define C_WRITE_ALGO_FLASH_D    30

#define C_WRITE_ALGO_FUSEmega0 40
#define C_WRITE_ALGO_NVMmega0 50

#define C_ALGO_CHIP_ERASE_D		100
#define C_ALGO_CHIP_ERASE_mega0 	120
struct mem
{
  uint32_t start;
  uint32_t wstart;		// address for write (for example - userrow need diffferent address for write)
  uint32_t size;
  uint32_t psize;
  uint8_t rw;			// 1 is rw
  uint8_t nvm;			// 1 NVM must be activated
  uint8_t not_affected_by_chip_erase;
  uint16_t write_algo;		//
  char *name;
};

struct mcu
{
  uint8_t sib[16];
  uint8_t ptr_bytes;		// NVM need 16 or 24 bits
  uint8_t if_locked_ignore_signature_test;	// set to 1 for AVR128DA  - if device is locked, signature can not be checked
  uint16_t chip_erase_algo;
  char signature[3];
  char *name;
  struct mem memory[100];	// we need lot of slots, for some memory arreas aliases can be set ..
};

// to allow multiple -U switches, create list of operations here
struct U
{
  char *name;
  struct mem *mem;
  char op;
  char *filename;
  char fmt;
  struct U *next;
};

struct prog
{
  char *serial_port;
  int serial_speed;
  struct U *U_first, *U_last;	// linked list for -U arguments
  struct U *u;			// pointer to current U
  int locked;
  int do_chip_erase;		// 0 no, any other value do chip erase
  int chip_erased;		// 1 if chip erase operation was succesfull
  int disable_autoerase;	// do not use page erase before flash programming
  char *part_name;		//
  struct mcu *mcu;		// mcu specification 
  struct mcu *mcu_table;	// all mcu spec
  int skip_signature_check;	//
  int skip_verify;		//
  uint32_t data_size;		// data size for this operation
  uint32_t page;		// used by programmers algo to set page number to program
  uint8_t *r;			// buffer for data to be read from MCU
  uint8_t *v;			// buffer for data to be send to MCU
};

// functions
int run_prog (struct prog *prog);	// base programmer code

int write_data_block (struct prog *prog);
int write_word_data_block (struct prog *prog);
int write_data_block_i (struct prog *prog);
int app_wait_UROWPROG_END ();
int app_wait_UROWPROG ();
int app_wait_CH_ERASE ();
int wait_NVM_IDLE (struct prog *prog);
int UPDI_wait_NVMPROG ();
void progressbar (int i, int max);
#endif

