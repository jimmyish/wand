# Change This to FreeBSD if you need to
OS=Linux

SUBDIR=lib Ethernet-Over-UDP clientsrc wand wansd
CFLAGS=-Iinclude/ -g -Wall -O3
CXXFLAGS=-Iinclude/ -g -Wall -O3

# Check that the install paths have been specified
DESTDIR ?= 

BINDIR ?= /usr/local/sbin
LIBDIR ?= /usr/local/lib
CONFDIR ?= /usr/local/etc
INITDIR ?= $(DESTDIR)/etc/init.d

ifeq ($(OS), Linux)
INSTALL=install -D
else
INSTALL=install -d
endif

all: 
	for i in $(SUBDIR); do \
		$(MAKE) OS=$(OS) -C $$i all; \
	done


clean:
	rm -f lib/*.o src/*~ include/*~
	for i in $(SUBDIR); do \
		$(MAKE) -C $$i clean; \
	done

install: all
	$(INSTALL) --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/Etud \
		$(DESTDIR)$(BINDIR)/Etud
	$(INSTALL) --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/ethertap.so \
		$(DESTDIR)$(LIBDIR)/wand/drivers/ethertap.so
	$(INSTALL) --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/tuntap.so \
		$(DESTDIR)$(LIBDIR)/wand/drivers/tuntap.so
	$(INSTALL) --group=root --mode=555 --owner=root \
		wand/wand \
		$(DESTDIR)$(BINDIR)/wand
	$(INSTALL) --group=root --mode=555 --owner=root \
		wansd/wansd \
		$(DESTDIR)$(BINDIR)/wansd
	$(INSTALL) --group=root --mode=644 --owner=root \
		misc/sample/etud.conf \
		$(DESTDIR)$(CONFDIR)/etud.conf.sample
	$(INSTALL) --group=root --mode=644 --owner=root \
		misc/sample/wand.conf \
		$(DESTDIR)$(CONFDIR)/wand.conf.sample
	$(INSTALL) --group=root --mode=555 --owner=root \
		clientsrc/client \
		$(DESTDIR)$(BINDIR)/Etudctl

install-debian:
	$(INSTALL) --group=root --mode=555 --owner=root \
		misc/debian/etc/init.d/Etud \
		$(INITDIR)/Etud
	$(INSTALL) --group=root --mode=555 --owner=root \
                misc/debian/etc/init.d/wand \
                $(INITDIR)/wand
	$(INSTALL) --group=root --mode=555 --owner=root \
                misc/debian/etc/init.d/wansd \
                $(INITDIR)/wansd
	sed -e "s@^ETUDCONF=.*@ETUDCONF=$(CONFDIR)/etud.conf@" \
	    -e "s@^DAEMON=.*@DAEMON=$(BINDIR)/Etud@" \
	    < $(INITDIR)/Etud > $(INITDIR)/Etud.tmp
	mv $(INITDIR)/Etud.tmp $(INITDIR)/Etud
	chmod 555 $(INITDIR)/Etud
	sed -e "s@^WAND_CONF=.*@WAND_CONF=$(CONFDIR)/wand.conf@" \
	    -e "s@^DAEMON=.*@DAEMON=$(BINDIR)/wand@" \
            < $(INITDIR)/wand > $(INITDIR)/wand.tmp
	mv $(INITDIR)/wand.tmp $(INITDIR)/wand
	chmod 555 $(INITDIR)/wand
	sed -e "s@^DAEMON=.*@DAEMON=$(BINDIR)/wansd@" \
            < $(INITDIR)/wansd > $(INITDIR)/wansd.tmp
	mv $(INITDIR)/wansd.tmp $(INITDIR)/wansd
	chmod 555 $(INITDIR)/wansd
