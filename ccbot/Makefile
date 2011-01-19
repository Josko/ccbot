SHELL = /bin/sh
SYSTEM = $(shell uname)
C++ = g++
CC = gcc
DFLAGS = -DSQLITE_THREADSAFE=0
OFLAGS = -O3
LFLAGS = -L. -L../bncsutil/src/bncsutil/ -lbncsutil -lpthread -ldl -lrt -lnsl -lcurses
CFLAGS = -Wall -Wextra

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

OBJS = bncsutilinterface.o bnet.o bnetprotocol.o chatevent.o config.o ccbot.o ccbotdb.o language.o socket.o util.o
COBJS = sqlite3.o
PROGS = ./ccbot++

all: $(OBJS) $(COBJS) $(PROGS)

./ccbot++: $(OBJS) $(COBJS)
	$(C++) -o ./ccbot++ $(OBJS) $(COBJS) $(LFLAGS)

clean:
	rm -f $(OBJS) $(COBJS) $(PROGS)
	
install: $(PROGS)
	cp $(PROGS) ..

$(OBJS): %.o: %.cpp
	$(C++) -o $@ $(CFLAGS) -c $<

$(COBJS): %.o: %.c
	$(CC) -o $@ $(CFLAGS) -c $<

./ccbot: $(OBJS) $(COBJS)
