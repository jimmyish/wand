CXXFLAGS=-Wall -g -DUSE_HASHMAP -O0
all:  wand

wand: tcp.o unix.o mainloop.o select.o wand.cc

clean:
	rm -f *~ *.o wand *.tab.c *.tab.h tokeniser.c