SUBDIR=lib Ethernet-Over-UDP clientsrc wand wansd
CFLAGS=-Iinclude/ -g -Wall -O3
CXXFLAGS=-Iinclude/ -g -Wall -O3

# Check that the destdir is set
DESTDIR ?= /usr/local
CONFDIR ?= /usr/local/etc

all: 
	for i in $(SUBDIR); do \
		$(MAKE) -C $$i all; \
	done


clean:
	rm -f lib/*.o src/*~ include/*~
	for i in $(SUBDIR); do \
		$(MAKE) -C $$i clean; \
	done

install: all
	install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/Etud \
		$(DESTDIR)/sbin/Etud
	install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/ethertap.so \
		$(DESTDIR)/lib/wand/drivers/ethertap.so
	install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/tuntap.so \
		$(DESTDIR)/lib/wand/drivers/tuntap.so
	install -D --group=root --mode=555 --owner=root \
		wand/wand \
		$(DESTDIR)/sbin/wand
	install -D --group=root --mode=555 --owner=root \
		wansd/wansd \
		$(DESTDIR)/sbin/wansd
	install -D --group=root --mode=644 --owner=root \
		misc/sample/etud.conf \
		$(CONFDIR)/etud.conf.sample
	install -D --group=root --mode=644 --owner=root \
		misc/sample/wand.conf \
		$(CONFDIR)/wand.conf.sample
	install -D --group=root --mode=555 --owner=root \
		clientsrc/client \
		$(DESTDIR)/sbin/Etudctl


