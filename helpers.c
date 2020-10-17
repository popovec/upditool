/*
    helpers.c

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

    various helpers routines

*/
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "UPDI_ll.h"
#include "UPDI_cmd.h"
#include "prog.h"
#include "helpers.h"

#if defined(WIN32) || defined(WIN64)
ssize_t getline(char **lineptr, size_t *n, FILE *stream);
#endif

struct mem *
get_1st_memory (struct mcu *mcu)
{
  if (mcu->memory[0].name[0] != '\0')
    return mcu->memory;
  else
    return NULL;

}

struct mem *
get_next_memory (struct mem *mem)
{
  mem++;
  if (mem->name[0] != '\0')
    return mem;
  else
    return NULL;
}

// NULL error
struct mem *
search_memory_by_name (struct mcu *mcu, char *name)
{
  struct mem *ret;
//  ret = get_1st_memory (mcu);
  for (ret = get_1st_memory (mcu); ret != NULL; ret = get_next_memory (ret))
    {
      if (0 == strcmp (name, ret->name))
	return ret;
    }
  return NULL;
}



static int
getnybble (char c)
{

  if (c < '0')
    return -1;
  if (c > '9')
    c = c - 7;
  if (c < '0')
    return -1;
  if (c > 0x40)
    return -1;
  return c - '0';
}

static int
getbyte (char *c)
{
  int n1, n2;

  if ((n1 = getnybble (c[0])) < 0)
    return -1;
  n1 = n1 << 4;
  if ((n2 = getnybble (c[1])) < 0)
    return -1;
  return n1 | n2;
}

// ---------------------------------------------------------------------------
//  IHEX

// based on https://en.wikipedia.org/wiki/Intel_HEX
int
ihex_read (FILE * f, uint8_t * buffer, int size)
{
  uint32_t base = 0;
  uint8_t data[255];
  uint8_t bc;
  uint32_t address;
  uint32_t max_addr = 0;
  uint8_t type;
  uint8_t chsum;
  int i, tmp;
  int end;
  int linec = 0;

  ssize_t nread;
  char *line = NULL;
  size_t len = 0;

  for (end = 0; end == 0;)
    {
      linec++;
      nread = getline (&line, &len, f);
      if (nread < 0)
	{
	  // EOF ?
	  if (errno == 0)
	    break;
	  fprintf (stderr, "Unable to read from file, error %s\n",
		   strerror (errno));
	  return -2;
	}
/* minimal characters on line:
:       1 - start code
00      2 - byte count (00 is allowed for example for record type 01 - end of file)
0000    4 - address (0..65535)
00      2 - record type
..      0 - no data bytes
xx      2 - checksumm
--------------------------
        11 bytes
of course + termination character = 12 bytes

*/
      if (nread < 12)
	{
	  fprintf (stderr,
		   "Unable to read from file, record too short (%d / %s)\n",
		   linec, line);
	  return -3;

	}
      // ihex record 1st char is ':'
      if (line[0] != ':')
	{
	  fprintf (stderr,
		   "Unable to read from file, 1st char of record is not :\n");
	  return -4;
	}
      if ((bc = getbyte (line + 1)) < 0)
	return -5;
      chsum = bc;
      if ((i = getbyte (line + 3)) < 0)
	return -6;
      chsum += i;
      if ((address = getbyte (line + 5)) < 0)
	return -7;
      chsum += address;
      address |= i << 8;
      if ((type = getbyte (line + 7)) < 0)
	return -8;
      chsum += type;
      if (nread < bc * 2 + 11)
	return -9;
      for (i = 0; i < bc; i++)
	{
	  if ((tmp = getbyte (line + 9 + 2 * i)) < 0)
	    return -10;
	  data[i] = tmp;
	  chsum += tmp;
	}
      if ((i = getbyte (line + 9 + 2 * i)) < 0)
	return -11;
      chsum += i;
      if (chsum != 0)
	return -12;
      switch (type)
	{
	case 0:
	  if (base + address + bc > size)
	    {
	      return -13;
	    }
	  if (max_addr < base + address + bc)
	    max_addr = base + address + bc;
	  memcpy (buffer + base + address, data, bc);
	  break;
	case 1:
	  end = 1;
	  break;
	case 2:
	  base = (data[0] << 8 | data[1]) << 4;
	  break;
	case 4:
	  base = (data[0] << 8 | data[1]) << 16;
	  break;
	case 3:
	case 5:
	  break;
	default:
	  return -14;
	}
    }
  free (line);
  return max_addr;
}

static void
ihex_write_block (FILE * f, uint8_t * buffer, int size)
{
  int i, j;
  int chsum;

  i = 0;
  while (size)
    {
      if (size > 16)
	{
	  chsum = 0x10 + (i >> 8) + (i & 0xff);
	  fprintf (f, ":10%04X00", i);
	  for (j = 0; j < 16; j++)
	    {
	      fprintf (f, "%02X", buffer[i]);
	      chsum += buffer[i++];
	    }
	  fprintf (f, "%02X\r\n", (0 - chsum) & 0xff);
	  size -= 16;
	}
      else
	{
	  chsum = size + (i >> 8) + (i & 0xff);
	  fprintf (f, ":%02X%04X00", size, i);
	  for (j = 0; j < size; j++)
	    {
	      fprintf (f, "%02X", buffer[i]);
	      chsum += buffer[i++];
	    }
	  fprintf (f, "%02X\r\n", (0 - chsum) & 0xff);
	  size = 0;
	}
    }
}

int
ihex_write (FILE * f, uint8_t * buffer, int size)
{
  int i, j;

  printf ("size=%d\n", size);

  for (j = 0, i = size;;)
    {
      if (i > 65536)
	{
	  ihex_write_block (f, buffer, 65536);
	  i -= 65536;
	  buffer += 65536;
	  j++;
	  fprintf (f, ":0200000400%02X%02X\r\n", j, 250 - j);
	}
      else
	{
	  ihex_write_block (f, buffer, i);
	  break;
	}
    }
  fprintf (f, ":00000001FF\r\n");
  return 0;
}

// ---------------------------------------------------------------------------
//  file read/write
int
file_read (char *filename, char fmt, uint8_t * buffer, uint32_t size)
{
  FILE *f;
  size_t vbytes = 0;

  if (size > MAX_MEMORY_LEN)
    size = MAX_MEMORY_LEN;

  memset (buffer, 0xff, MAX_MEMORY_LEN);

  printf ("format %c\n", fmt);
  if (fmt == 'm')
    {
      char *c = strtok (filename, " ,");
      char *n;
      unsigned long v;

      while ((c != NULL) && size--)
	{
	  v = strtoul (c, &n, 0);
	  if (*n != '\0')
	    {
	      fprintf (stderr, "invalid value\n");
	      return -1;
	    }
	  c = strtok (NULL, " ,");
	  *(buffer++) = v & 0xff;
	  vbytes++;
	}
      return vbytes;
    }

  f = fopen (filename, "r");
  if (f == NULL)
    {
      fprintf (stderr, "Unable to open file %s for read\n", filename);
      return -1;
    }
  vbytes = -1;
  if (fmt == 'r')
    vbytes = fread (buffer, 1, size, f);
  else if (fmt == 'i')
    vbytes = ihex_read (f, buffer, size);
  else
    fprintf (stderr, "Unknown format\n");

  if (vbytes < 1)
    vbytes = -1;
  fclose (f);
  return vbytes;
}

int
file_write (char *filename, char fmt, uint8_t * buffer, uint32_t size)
{
  uint8_t *b = buffer;
  uint32_t s = size;
  size_t vbytes = 0;
  int ret = size;

  FILE *f = NULL;

  if (0 == strcmp ("-", filename))
    f = stdout;
  else
    f = fopen (filename, "w");

  if (f == NULL)
    return -1;

  switch (fmt)
    {
    case 'i':			// intel hex
      if (ihex_write (f, buffer, size))
	ret = -1;
      else
	ret = size;
      break;

//  case 's':         // srecord
//  case 'b':         // binary
//  case 'd':         // decimal
//  case 'o':         // octal
    case 'h':			// hexa
      while (size--)
	{
	  fprintf (f, "0x%02x", *(b++));
	  if (size)
	    fprintf (f, ",");
	  vbytes++;
	}
      size = vbytes;
      printf ("\n");
      break;

    case 'r':
      if (fwrite (b, s, 1, f) != 1)
	ret = -1;
      break;
    default:
      fprintf (stderr, "Unknown format\n");
      ret = -1;
    }

  if (f != stdout)
    fclose (f);

  printf ("File write OK, size=%d\n", size);
  return ret;
}
