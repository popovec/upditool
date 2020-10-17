/*
    UPDI_ll.h

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

    UPDI low level - serial port
*/

#ifndef __UPDI_ll__
#define __UPDI_ll__


int updi_serial_init (char * serial);
int updi_set_speed(int speed);
int updi_send_break (void);
void updi_serial_close();

int updi_transaction (uint8_t * send, uint16_t s_len,uint8_t * receive, uint16_t r_len, int timeout);
#endif
