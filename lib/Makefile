CXXFLAGS=-I../include
CFLAGS=-I../include
TESTS=$(wildcard tests/*.c)
SUCCESS=$(addsuffix .test,$(basename $(TESTS)))

all: libconfig.a daemons.o protoverlay.o test

libconfig.a:  \
	     libconfig.a(config.parser.o) \
	     libconfig.a(libconfig.o) \
	     libconfig.a(config.lexer.o)
	ranlib $@

test: $(SUCCESS) libconfig.a

clean:
	rm -f libconfig.a tests/*.test *.o config.h config.parser.h

%.parser.c: %.y
	bison -d -o$@ $<

%.lexer.c: %.l
	flex -o$@ $<

%.test: %.o libconfig.a
	gcc $(CFLAGS) $< -L. -lconfig -lfl -o $(basename $<)
	$(basename $<) $(basename $<).in 

