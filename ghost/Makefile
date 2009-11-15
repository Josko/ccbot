SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS =
OFLAGS = -O3
LFLAGS = -L. -L../bncsutil/src/bncsutil/ -lbncsutil -lpthread -ldl -lrt -lnsl
CFLAGS =

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
else
LFLAGS += -lrt
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket -lnsl
endif

CFLAGS += $(OFLAGS) $(DFLAGS) -I. -I../bncsutil/src/

OBJS = bncsutilinterface.o bnet.o bnetprotocol.o chatevent.o commandpacket.o config.o ghost.o ghostdb.o ghostdbsqlite.o language.o socket.o util.o 
COBJS = sqlite3.o
PROGS = ./ghost++

all: $(OBJS) $(COBJS) $(PROGS)

./ghost++: $(OBJS) $(COBJS)
	$(C++) -o ./ghost++ $(OBJS) $(COBJS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

./ghost++: $(OBJS) $(COBJS)

all: $(PROGS)

bncsutilinterface.o: ghost.h util.h bncsutilinterface.h
bnet.o: ghost.h util.h config.h language.h socket.h commandpacket.h ghostdb.h bncsutilinterface.h bnetprotocol.h bnet.h
bnetprotocol.o: ghost.h util.h bnetprotocol.h
commandpacket.o: ghost.h commandpacket.h
config.o: ghost.h config.h
ghost.o: ghost.h util.h config.h language.h socket.h ghostdb.h ghostdbsqlite.h bnet.h
ghostdb.o: ghost.h config.h ghostdb.h
ghostdbsqlite.o: ghost.h util.h config.h ghostdb.h ghostdbsqlite.h
language.o: ghost.h config.h language.h
socket.o: ghost.h util.h socket.h
util.o: ghost.h util.h
chatevent.o: ghost.h util.h config.h language.h socket.h commandpacket.h ghostdb.h bncsutilinterface.h bnetprotocol.h bnet.h
