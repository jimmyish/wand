SUBDIR=lib Ethernet-Over-UDP clientsrc wand wansd
CFLAGS=-Iinclude/ -g -Wall -O3
CXXFLAGS=-Iinclude/ -g -Wall -O3

# Check that the Base Directory is set
BASEDIR ?= 
# Check that install paths relative to the base directory are set
BINDIR ?= /usr/local
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
		$(BASEDIR)$(BINDIR)/sbin/Etud
	install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/ethertap.so \
		$(BASEDIR)$(BINDIR)/lib/wand/drivers/ethertap.so
	install -D --group=root --mode=555 --owner=root \
		Ethernet-Over-UDP/drivers/tuntap.so \
		$(BASEDIR)$(BINDIR)/lib/wand/drivers/tuntap.so
	install -D --group=root --mode=555 --owner=root \
		wand/wand \
		$(BASEDIR)$(BINDIR)/sbin/wand
	install -D --group=root --mode=555 --owner=root \
		wansd/wansd \
		$(BASEDIR)$(BINDIR)/sbin/wansd
	install -D --group=root --mode=644 --owner=root \
		misc/sample/etud.conf \
		$(BASEDIR)$(CONFDIR)/etud.conf.sample
	install -D --group=root --mode=644 --owner=root \
		misc/sample/wand.conf \
		$(BASEDIR)$(CONFDIR)/wand.conf.sample
	install -D --group=root --mode=555 --owner=root \
		clientsrc/client \
		$(BASEDIR)$(BINDIR)/sbin/Etudctl

install-debian:
	install -D --group=root --mode=555 --owner=root \
		misc/debian/etc/init.d/Etud \
		$(BASEDIR)/etc/init.d/Etud
	install -D --group=root --mode=555 --owner=root \
                misc/debian/etc/init.d/wand \
                $(BASEDIR)/etc/init.d/wand
	install -D --group=root --mode=555 --owner=root \
                misc/debian/etc/init.d/wansd \
                $(BASEDIR)/etc/init.d/wansd
	sed -e "s@^ETUDCONF=.*@ETUDCONF=$(BASEDIR)$(CONFDIR)/etud.conf@" \
	    -e "s@^DAEMON=.*@DAEMON=$(BASEDIR)$(BINDIR)/sbin/Etud@" \
	    < $(BASEDIR)/etc/init.d/Etud > $(BASEDIR)/etc/init.d/Etud.tmp
	mv $(BASEDIR)/etc/init.d/Etud.tmp $(BASEDIR)/etc/init.d/Etud
	chmod 555 $(BASEDIR)/etc/init.d/Etud
	sed -e "s@^WAND_CONF=.*@WAND_CONF=$(BASEDIR)$(CONFDIR)/wand.conf@" \
	    -e "s@^DAEMON=.*@DAEMON=$(BASEDIR)$(BINDIR)/sbin/wand@" \
            < $(BASEDIR)/etc/init.d/wand > $(BASEDIR)/etc/init.d/wand.tmp
	mv $(BASEDIR)/etc/init.d/wand.tmp $(BASEDIR)/etc/init.d/wand
	chmod 555 $(BASEDIR)/etc/init.d/wand
	sed -e "s@^DAEMON=.*@DAEMON=$(BASEDIR)$(BINDIR)/sbin/wansd@" \
            < $(BASEDIR)/etc/init.d/wansd > $(BASEDIR)/etc/init.d/wansd.tmp
	mv $(BASEDIR)/etc/init.d/wansd.tmp $(BASEDIR)/etc/init.d/wansd
	chmod 555 $(BASEDIR)/etc/init.d/wansd
