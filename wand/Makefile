CXXFLAGS=-Wall -g -DUSE_HASHMAP -O0
all:  wand protocol

grammar.tab.c grammar.tab.h: grammar.y
	bison -d grammar.y

tokeniser.c: tokeniser.l grammar.tab.h
	flex -t tokeniser.l >tokeniser.c

protocol: tokeniser.o grammar.tab.o protocol.o
#$(CC) $(CFLAGS) tokeniser.o grammar.tab.o -o protocol 

wand: tcp.o unix.o mainloop.o select.o wand.cc

clean:
	rm -f *~ *.o wand *.tab.c *.tab.h tokeniser.c