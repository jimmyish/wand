OS=Linux
CFLAGS=-Iinclude/ -I../include/ -I/usr/local/include -g -Wall 
CXXFLAGS=-Iinclude/ -I../include/ -I/usr/local/include -g -Wall
LDFLAGS=-L../lib
DRIVERS=drivers/ethertap.so

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

all: Etud $(DRIVERS) test

Etud: src/Etud.o src/interfaces.o src/list.o src/udp.o src/ui.o \
	src/mainloop.o ../lib/daemons.o ../lib/debug.o
	$(CXX) $(LDFLAGS) -rdynamic $^ $(LIBS) -o Etud

test: src/list-test
	@echo Executing tests...
	src/list-test 

drivers/ethertap.so: drivers/ethertap.c

drivers/tuntap.so: drivers/tuntap.c

include/ethertap_cfg.h:
	cp include/ethertap_cfg.h.sample include/ethertap_cfg.h

drivers/%.o: drivers/%.c
	$(CC) $(CFLAGS) -fPIC -c $^ -o $@

src/%-test: src/%.cc ../lib/debug.o ../lib/daemons.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -DTEST $^ -o $@

drivers/%.so: drivers/%.o
	ld -x --shared $< -o $@

clean:
	rm -f drivers/*{.o,.so,~} *~ src/*{.o,~} src/*-test


.PHONY: clean all test
