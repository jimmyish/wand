SUBDIR=Ethernet-Over-UDP clientsrc wand wansd

all:
	for i in $(SUBDIR); do \
		$(MAKE) -C $$i all; \
	done

clean:
	for i in $(SUBDIR); do \
		$(MAKE) -C $$i clean; \
	done
