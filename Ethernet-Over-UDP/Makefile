CFLAGS=-Iinclude/ -I../include/ -g -Wall 
CXXFLAGS=-Iinclude/ -I../include/ -g -Wall #-DINSANE_LIST_DEBUG

all: Etud drivers/ethertap.so test

Etud: src/Etud.o src/interfaces.o src/list.o src/udp.o src/ui.o \
	src/mainloop.o ../lib/daemons.o 
	$(CXX) $(LDFLAGS) -rdynamic $^ -ldl -o Etud

test: src/list-test
	@echo Executing tests...
	src/list-test 

drivers/ethertap.so: drivers/ethertap.c

include/ethertap_cfg.h:
	cp include/ethertap_cfg.h.sample include/ethertap_cfg.h

drivers/%.o: drivers/%.c
	$(CC) $(CFLAGS) -fPIC -c $^ -o $@

src/%-test: src/%.cc
	$(CXX) $(CXXFLAGS) -DTEST $^ -o $@

drivers/%.so: drivers/%.o
	ld -x --shared $< -o $@

clean:
	rm -f drivers/*{.o,.so,~} *~ src/*{.o,~} src/*-test


.PHONY: clean all test
