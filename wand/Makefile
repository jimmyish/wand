CFLAGS=-Iinclude/ -I../include/ -g -Wall
CXXFLAGS=-Iinclude/ -I../include/ -g -Wall #-DINSANE_LIST_DEBUG
LDFLAGS=-L../lib

ifeq ($(OS),Linux)
DRIVERS+=drivers/tuntap.so
LIBS=-ldl -lconfig -lfl
CFLAGS+=-DLINUX
CXXFLAGS+=-DLINUX
else
LIBS=-lconfig -lfl
CFLAGS+=-DFREEBSD
CXXFLAGS+=-DFREEBSD
endif

all: wand

wand: src/wand.o ../lib/daemons.o ../lib/debug.o ../lib/protoverlay.o
	$(CXX) $(LDFLAGS) -rdynamic $^ $(LIBS) -o wand

clean:
	rm -f wand src/*.o src/*~
