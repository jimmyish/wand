CC=gcc
CFLAGS=-g -lpq -Wall -I../include/

ifeq ($(OS),Linux)
CFLAGS+=-DLINUX
else
CFLAGS+=-DFREEBSD
endif

OBJS=../lib/daemons.o

all: wansd

clean:
	rm -f wansd *.o *~
wansd: wansd-pgsql.c $(OBJS)
	$(CC) $(CFLAGS) -c $< -o wansd
