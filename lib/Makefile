CXXFLAGS=-I../include
CFLAGS=-I../include
TESTS=$(wildcard tests/*.c)
SUCCESS=$(addsuffix .test,$(basename $(TESTS)))

all: libconfig.a daemons.o protoverlay.o test

libconfig.a:  \
	     libconfig.a(config.o) \
	     libconfig.a(libconfig.o) \
	     libconfig.a(config.yy.o)
	ranlib $@

test: $(SUCCESS) libconfig.a

clean:
	rm -f libconfig.a tests/*.test *.o config.h

%.c %.h: %.y
	bison -d -o$@ $<

%.yy.c: %.l %.h
	flex -o$@ $<

%.test: %.o libconfig.a
	gcc $(CFLAGS) $< -L. -lconfig -lfl -o $(basename $<) 
	$(basename $<) $(basename $<).in 

