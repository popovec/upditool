/*
    upditool.c

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

    collect data drom user (commandline switches etc) and data from
    mcu database, create input structure for programmer
    
*/
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "prog.h"
#include "helpers.h"

// find MCU parameters (based on -p switch)
// 0 OK
static int
search_mcu_spec (struct prog *prog)
{
  struct mcu *mcu;

  if (prog->part_name == NULL)
    {
      fprintf (stderr,
	       "Please specify part by -p switch (-p help to list available devices)\n");
      return -1;
    }
  for (mcu = prog->mcu_table; mcu->name[0] != '\0'; mcu++)
    {
      if (0 == strcmp (prog->part_name, mcu->name))
	{
	  prog->mcu = mcu;
	  return 0;
	}
    }
  fprintf (stderr, "Unknonwn device %s, available device names:\n",
	   prog->part_name);
  for (mcu = prog->mcu_table; mcu->name[0] != '\0'; mcu++)
    fprintf (stderr, "'%s'\n", mcu->name);
  return -1;
}

// parse/check all -U arguments
// 0 OK
static int
parse_U (struct prog *prog)
{
  struct U *u;
  struct mem *mem;
  char *tmp;

  for (u = prog->U_first; u != NULL; u = u->next)
    {
      // here full -U optarg is copied, extract memory name here
      tmp = strchr (u->name, ':');
      if (NULL == tmp)
	{
	  fprintf (stderr, "unable to parse memory type\n");
	  return -1;
	}
      *(tmp) = '\0';
      mem = search_memory_by_name (prog->mcu, u->name);
      if (NULL == mem)
	{
	  fprintf (stderr,
		   "Unable to found '%s' memory on device %s, available memories:\n",
		   u->name, prog->mcu->name);


	  for (mem = get_1st_memory (prog->mcu); mem != NULL;
	       mem = get_next_memory (mem))
	    fprintf (stderr,
		     "'%s'%s is %s, start at: 0x%06x size % 7d %s\n",
		     mem->name,
		     strlen (mem->name) >= 6 ? "\t" : "\t\t",
		     mem->rw ? "RW" : "RO",
		     mem->start, mem->size,
		     mem->rw ? (mem->write_algo ==
				C_WRITE_ALGO_NONE ?
				"(Write not implemented for now)" : "") : "");

	  return -1;
	}
      u->mem = mem;

      tmp++;
      u->op = *(tmp);

      tmp++;
      if (*(tmp) != ':')
	{
	  fprintf (stderr,
		   "unable to parse filename for memory operation, missing collon?\n");
	  return -1;
	}
      tmp++;
      u->filename = strdup (tmp);
      tmp = strchr (u->filename, ':');
      if (NULL != tmp)
	{
	  *(tmp) = '\0';
	  tmp++;
	  u->fmt = *(tmp);
	}
      if (strlen (u->filename) < 1)
	{
	  fprintf (stderr, "unable to parse filename\n");
	  return -1;
	}
      switch (u->op)
	{
	case 'w':
	  if (mem->rw != 1)
	    {
	      fprintf (stderr,
		       "Unable to write to memory %s, this memory is read only\n",
		       u->name);
	      return -1;
	    }
	  break;
	case 'r':
	  if (u->fmt == 'm')
	    {
	      fprintf (stderr,
		       "specified format 'm' can ot be used for read operation\n");
	      return -1;
	    }
	case 'v':
	  break;
	default:
	  fprintf (stderr, "Unknown operation '%c'\n", u->op);
	  return -1;
	}
    }
  return 0;
}

int
main (int argc, char *argv[])
{
  struct prog p, *prog = &p;

// array for memory read/write
  uint8_t r[MAX_MEMORY_LEN + 10];	// raw dat from device
  uint8_t v[MAX_MEMORY_LEN + 10];	// verify data form file
  int opt;
  struct U *u;

// initialize prog structure, load static MCU specification...
  struct mcu mcu_table[] = {	//
#include "device_avr.h"
    {
     .name = "",
     },
  };
  memset (prog, 0, sizeof (struct prog));
  prog->serial_speed = 115200;	// default serial port speed
  prog->mcu_table = mcu_table;
// 
  prog->r = r;
  prog->v = v;

  while ((opt = getopt (argc, argv, "hVFeb:p:P:U:")) != -1)
    {
      switch (opt)
	{
	case 'h':
	  {
	    fprintf (stderr,	//
		     "Options:\n");
	    fprintf (stderr,	//
		     "  -p <partno>                Required, Specify AVR device.\n");
	    fprintf (stderr,	//
		     "  -b <baudrate>              default 115200\n");
	    fprintf (stderr,	//
		     "  -D                         Disable auto erase for flash memory (TODO)\n");
	    fprintf (stderr,	//
		     "  -P <port>                  Required, Specify connection port.\n");
	    fprintf (stderr,	//
		     "  -F                         Disable signature check.\n");
	    fprintf (stderr,	//
		     "  -V                         Disable verify.\n");
	    fprintf (stderr,	//
		     "  -e                         Perform a chip erase.\n");
	    fprintf (stderr,	//
		     "  -U <memtype>:r|w|v:<filename>[:format]> \n");
	    fprintf (stderr,	//
		     "                             Memory operation specification.\n");
	    fprintf (stderr,	//
		     "                             multiple -U options are allowed\n");
	    fprintf (stderr,	//
		     "(Warning this code was tested only on avr128DA32 and megaAVR 4808)\n");
	    return 0;
	  }
	case 'e':
	  prog->do_chip_erase = 1;
	  break;
	case 'b':
	  prog->serial_speed = atoi (optarg);
	  if (prog->serial_speed < 75000)
	    fprintf (stderr,
		     "Warning, requested speed below recommended speed for UPDI\n");
	  if (prog->serial_speed > 16000000)
	    fprintf (stderr,
		     "Warning, requested speed over recommended speed for UPDI\n");
	  break;
	case 'F':
	  prog->skip_signature_check = 1;
	  break;
	case 'V':
	  prog->skip_verify = 1;
	  break;
	case 'D':
	  prog->disable_autoerase = 1;
	  break;
	case 'P':
	  if (prog->serial_port)
	    free (prog->serial_port);
	  prog->serial_port = strdup (optarg);
	  break;
	case 'p':
	  if (prog->part_name != NULL)
	    {
	      fprintf (stderr, "Multiple -p switches ?\n");
	      exit (EXIT_FAILURE);
	    }
	  else
	    prog->part_name = strdup (optarg);
	  break;
	case 'U':
	  // -U memtype:op:filename[:format]
	  // create linked list here, we can't check the memory name, because the device can be specified later with the -p switch
	  u = calloc (sizeof (struct U), 1);
	  if (u == NULL)
	    exit (1);
	  if (prog->U_first == NULL)
	    prog->U_first = u;
	  else
	    prog->U_last->next = u;
	  prog->U_last = u;
	  u->name = strdup (optarg);
	  u->fmt = 'r';		// default raw format
	  break;
	default:
	  fprintf (stderr, "Usage: %s [-P serial port] [-p partname] \n",
		   argv[0]);
	  exit (EXIT_FAILURE);
	}
    }
  if (prog->serial_port == NULL)
    {
      fprintf (stderr,
	       "Please specify serial port by -P  switch, for example /dev/ttyUSB2 .. \n");
      exit (EXIT_FAILURE);
    }

// search for device parameters
  if (search_mcu_spec (prog))
    exit (EXIT_FAILURE);

// parse/check all -U arguments
  if (parse_U (prog))
    exit (EXIT_FAILURE);

  return run_prog (prog);
}
