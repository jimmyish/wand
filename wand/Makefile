CFLAGS=-Iinclude/ -I../include/ -g -Wall
CXXFLAGS=-Iinclude/ -I../include/ -g -Wall #-DINSANE_LIST_DEBUG
LDFLAGS=-L../lib

all: wand

wand: src/wand.o ../lib/daemons.o ../lib/debug.o ../lib/protoverlay.o
	$(CXX) $(LDFLAGS) -rdynamic $^ -ldl -lconfig -lfl -o wand

clean:
	rm -f wand src/*.o src/*~
