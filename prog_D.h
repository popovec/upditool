/*
    prog_D.h

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

 header file

 AVR D series specific code - tested on AVR128DA32

 128 KB AVR128DA28 AVR128DA32 AVR128DA48 AVR128DA64

 may work on MCU with smaller memories:
 64 KB  AVR64DA28 AVR64DA32  AVR64DA48   AVR64DA64
 32 KB  AVR32DA28 AVR32DA32  AVR32DA48


 may work on DB series..

*/
// 0 OK
int write_UROW_D (struct prog *prog);
// 0 ok
// any other value error 
int chip_erase_D ();

int write_NVM_EEPROM_D (struct prog *prog);

int write_flash_D (struct prog *prog);