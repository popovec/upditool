/*
    helpers.h

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

    various helpers routines (header file)
*/

// NULL error
struct mem * search_memory_by_name (struct mcu *mcu, char *name);
int file_read (char *filename, char fmt, uint8_t * buffer, uint32_t size);
int file_write (char *filename, char fmt, uint8_t * buffer, uint32_t size);

struct mem * get_1st_memory (struct mcu *mcu);
struct mem * get_next_memory (struct mem *mem);


