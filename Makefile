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
	@echo -n Installing Etud...
	@install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/Etud \
		/usr/local/sbin/Etud && echo Done
	@echo -n Installing Drivers...
	@install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/ethertap.so \
		/usr/local/lib/wand/drivers/ethertap.so && echo Done
	@echo -n Installing wand_startup.sh...
	@install -D --group=root --mode=555 --owner=root \
		startup.sh \
		/usr/local/sbin/wand_startup.sh && echo Done
	@echo -n Installing wand...
	@install -D --group=root --mode=555 --owner=root \
		wand/wand \
		/usr/local/sbin/wand && echo Done
	@echo -n Installing wansd...
	@install -D --group=root --mode=555 --owner=root \
		wansd/wansd \
		/usr/local/sbin/wansd && echo Done
	@echo -n Installing sample configuration...
	@install -D --group=root --mode=644 --owner=root \
		wand.conf.original \
		/usr/local/etc/wand.$$(hostname).conf.original && echo Done
	@echo -n Installing client program...
	@install -D --group=root --mode=555 --owner=root \
		clientsrc/client \
		/usr/local/sbin/Etudctl && echo Done


