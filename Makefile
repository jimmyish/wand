SUBDIR=lib Ethernet-Over-UDP clientsrc wand wansd
CFLAGS=-Iinclude/ -g -Wall -O3
CXXFLAGS=-Iinclude/ -g -Wall -O3

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
		/usr/local/sbin/Etud
	install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/ethertap.so \
		/usr/local/lib/wand/drivers/ethertap.so
	install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/tuntap.so \
		/usr/local/lib/wand/drivers/tuntap.so
	install -D --group=root --mode=555 --owner=root \
		startup.sh \
		/usr/local/sbin/wand_startup.sh
	install -D --group=root --mode=555 --owner=root \
		wand/wand \
		/usr/local/sbin/wand
	install -D --group=root --mode=555 --owner=root \
		wansd/wansd \
		/usr/local/sbin/wansd
	install -D --group=root --mode=644 --owner=root \
		misc/sample/etud.conf \
		/usr/local/etc/etud.conf.sample
	install -D --group=root --mode=644 --owner=root \
		misc/sample/wand.conf \
		/usr/local/etc/wand.conf.sample
	install -D --group=root --mode=555 --owner=root \
		clientsrc/client \
		/usr/local/sbin/Etudctl


