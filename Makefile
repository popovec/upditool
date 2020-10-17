all:	upditool

CC=gcc

UPDI_ll.o:	UPDI_ll.c UPDI_ll.h
		$(CC) -Wall UPDI_ll.c -o UPDI_ll.o -c -g

UPDI_cmd.o:	UPDI_ll.h UPDI_cmd.h UPDI_cmd.c
		$(CC) -Wall UPDI_cmd.c -c -o UPDI_cmd.o -g

prog.o:	prog.h prog.c helpers.h prog_D.h
		$(CC) -Wall prog.c -c -o prog.o

prog_D.o:	prog_D.c prog_D.h prog.h
		$(CC) -Wall prog_D.c -c -o prog_D.o

prog_mega0.o:	prog_mega0.c prog_mega0.h 
		$(CC) -Wall prog_mega0.c -c -o prog_mega0.o

helpers.o:	helpers.c helpers.h
		$(CC) -Wall helpers.c -c -o helpers.o
		
upditool.o:	upditool.c UPDI_ll.h UPDI_cmd.h device_avr.h helpers.h prog.h
		$(CC) -Wall upditool.c -c -o upditool.o

getline.o:	getline.c
		$(CC) -Wall getline.c -c -o getline.o

upditool:	upditool.o UPDI_ll.o UPDI_cmd.o prog.o helpers.o prog_D.o prog_mega0.o
		$(CC) -Wall UPDI_ll.o UPDI_cmd.o upditool.o prog.o prog_D.o prog_mega0.o helpers.o -g -o upditool

install:	upditool
		mkdir -p $(DESTDIR)/usr/bin/
		cp upditool $(DESTDIR)/usr/bin/

clean:
	rm -f *~
	rm -f *.o
	rm -f upditool
		