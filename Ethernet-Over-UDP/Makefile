CFLAGS=-Iinclude/
CXXFLAGS=-Iinclude/

all: Etud drivers/ethertap.so

Etud: src/Etud.o src/interfaces.o src/list.o src/udp.o
	$(CXX) $(LDFLAGS) -rdynamic $^ -ldl -o Etud

drivers/ethertap.so: drivers/ethertap.c

drivers/%.o: drivers/%.c
	$(CC) $(CFLAGS) -fPIC -c $^ -o $@

drivers/%.so: drivers/%.o
	ld -x --shared $< -o $@

clean:
	rm -f drivers/*{.o,.so,~} *~ src/*{.o,~}


.PHONY: clean all