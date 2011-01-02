SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS = -DSQLITE_THREADSAFE=0
OFLAGS = -O3
LFLAGS = -L. -L../bncsutil/src/bncsutil/ -lbncsutil -lpthread -ldl -lrt -lnsl -lcurses
CFLAGS = -Wall

ifeq ($(SYSTEM),Darwin)
DFLAGS += -D__APPLE__
OFLAGS += -flat_namespace
endif

ifeq ($(SYSTEM),FreeBSD)
DFLAGS += -D__FREEBSD__
endif

ifeq ($(SYSTEM),SunOS)
DFLAGS += -D__SOLARIS__
LFLAGS += -lresolv -lsocket
endif

CFLAGS += $(OFLAGS) $(DFLAGS) -I. -I../bncsutil/src/

OBJS = bncsutilinterface.o bnet.o bnetprotocol.o chatevent.o commandpacket.o config.o ccbot.o ccbotdb.o language.o socket.o util.o
COBJS = sqlite3.o
PROGS = ./ccbot++

all: $(OBJS) $(COBJS) $(PROGS)

./ccbot++: $(OBJS) $(COBJS)
	$(C++) -o ./ccbot++ $(OBJS) $(COBJS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

./ccbot: $(OBJS) $(COBJS)

all: $(PROGS)

bncsutilinterface.o: ccbot.h util.h bncsutilinterface.h
bnet.o: ccbot.h util.h config.h language.h socket.h commandpacket.h ccbotdb.h bncsutilinterface.h bnetprotocol.h bnet.h
bnetprotocol.o: ccbot.h util.h bnetprotocol.h
commandpacket.o: ccbot.h commandpacket.h
config.o: ccbot.h config.h
ccbot.o: ccbot.h util.h config.h language.h socket.h ccbotdb.h bnet.h
ccbotdb.o: ccbot.h config.h ccbotdb.h
ccbotdbsqlite.o: ccbot.h util.h config.h ccbotdb.h
language.o: ccbot.h config.h language.h
socket.o: ccbot.h util.h socket.h
util.o: ccbot.h util.h
chatevent.o: ccbot.h util.h config.h language.h socket.h commandpacket.h ccbotdb.h bncsutilinterface.h bnetprotocol.h bnet.h