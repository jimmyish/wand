CFLAGS=-g -Wall -I../include/

ifeq ($(OS),Linux)
CFLAGS+=-DLINUX
else
CFLAGS+=-DFREEBSD
endif

all: wansd

clean:
	rm -f wansd *.o *~
wansd: wansd.c ../lib/daemons.o
