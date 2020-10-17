/*
    UPDI_cmd.h

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

#ifndef __UPDI_cmd__
#define __UPDI_cmd__
#include <stdint.h>
// -1 error
// 0 to 255 normal return value

int UPDI_cmd_LDCS (uint8_t addr);

// -1 error
// 0 OK
//
int UPDI_cmd_STCS (uint8_t addr, uint8_t data);


// store pointer to UPDI (variants for 24/16 bit UPDI)
// 0 OK
int cmd_ST16_to_PTR (uint32_t addr);
int cmd_ST24_to_PTR (uint32_t addr);

// -1 error
// 0 OK
//
int UPDI_cmd_LD_BYTE (uint8_t * buffer, int count);

// -1 error
// 0 OK
//
int UPDI_cmd_ST_BYTE (uint8_t * buffer, int count);

int UPDI_cmd_ST_BYTE_RSD (uint8_t * buffer, int count);
int UPDI_cmd_ST_WORD_RSD (uint8_t * buffer, int count);

// -1 transaction error
// 0 OK
// 1 timeout
int UPDI_do_SYSTEM_RESET ();

// test (mask) in  ASI Key Status (UPDI register 7), if key is not accepted
// send key (repeat, then tiemout)
// 0 OK
// -1 fatal error (transaction, key too long etc.)
// 1 timeout
int UPDI_cmd_KEY (uint8_t *key,uint8_t size,uint8_t mask);


int UPDI_cmd_KEY_NVM_PROG ();
int UPDI_cmd_KEY_CHIP_ERASE ();
int UPDI_cmd_KEY_USERROW ();

// read SIB into buffer
int UPDI_cmd_READ_SIB (uint8_t * buffer);

int UPDI_NVM_enable ();


#endif
