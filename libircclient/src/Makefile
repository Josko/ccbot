# $Id: Makefile.in 62 2005-03-09 21:11:53Z gyunaev $
CC = gcc
CFLAGS = -Wall -DIN_BUILDING_LIBIRC -O3 -DENABLE_THREADS -D_REENTRANT
AR=ar cr
RANLIB=ranlib
INCLUDES=-I../include


OBJS = libircclient.o

all:	lib

lib:	libircclient.a

install: lib
	-mkdir /usr/local/include
	-mkdir /usr/local/lib
	cp ../include/libircclient.h /usr/local/include/libircclient.h
	cp ../include/libirc_errors.h /usr/local/include/libirc_errors.h
	cp ../include/libirc_events.h  /usr/local/include/libirc_events.h 
	cp libirc.a /usr/local/include/lib/libirc.a

$(OBJS): utils.c dcc.c errors.c portable.c sockets.c colors.c

libircclient.a: $(OBJS)
	$(AR) libircclient.a $(OBJS)
	$(RANLIB) libircclient.a

clean:
	rm -f libircclient.a $(OBJS)

distclean: clean
	-rm -f Makefile

.c.o:
	@echo "Compiling  $<"
	@$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<
