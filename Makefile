SUBDIR=Ethernet-Over-UDP clientsrc wand wansd
CFLAGS=-Iinclude/ -g -Wall -O3
CXXFLAGS=-Iinclude/ -g -Wall -O3

all: lib/daemons.o
	for i in $(SUBDIR); do \
		$(MAKE) -C $$i all; \
	done

lib/daemons.o: src/daemons.cc include/daemons.h
	$(CXX) $(CXXFLAGS) -c src/daemons.cc -o lib/daemons.o

clean:
	rm -f lib/*.o src/*~ include/*~
	for i in $(SUBDIR); do \
		$(MAKE) -C $$i clean; \
	done
