CXXFLAGS=-I../include
CFLAGS=-I../include

all: config.a daemons.o protoverlay.o

config.a: config.a(config.tab.o) config.a(lex.yy.o) config.a(config.o)

config.tab.c config.tab.h: config.y
	bison -d config.y

lex.yy.c: config.l config.tab.h
	flex config.l

clean:
	rm -f config.tab.{h,c} lex.yy.c *.o config.a
