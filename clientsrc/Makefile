
ifeq ($(OS),Linux)
CXXFLAGS=-O3 -g -Wall -I../include/
else
CXXFLAGS=-O3 -g -Wall -L/usr/local/lib -lgnugetopt -I/usr/local/include -I/usr/include -I../include/
endif

all: client

client: client.cc ../lib/protoverlay.o

clean:
	rm -f *~ core *.o client
