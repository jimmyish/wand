/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

/*
 *	ethertap.c
 *	Originally ipt by:  cmos	James Spooner  <james@spoons.gen.nz>
 *      Hacked for WANd by: Isomer	Perry Lorier   <isomer@coders.net>
 *
 *	read/write/setup the ETHERTAP linux device.
 *
 * TODO:
 *  Restore all flags on link 'down'.
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fcntl.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <sys/socket.h>

#include "ethertap_cfg.h" /* grab max and default /dev/tapX numbers */
#include "driver.h"

static int fd = -1;
static int tapdevno;

static int ethertap_setup(unsigned long myid) 
{
	char tapdevice[16];
	printf("ethertap_setup( %lu ) entered...\n", myid);

	fd = 0;	
	tapdevno = FIRST_TAP_NUMBER;
	
	while (tapdevno<MAX_ETHERTAP_DEVICES) {
		snprintf(tapdevice, 16, "/dev/tap%d", tapdevno);
		fd = open(tapdevice, O_RDWR);
		if( fd >= 0 ) {
			return fd;
		}
		tapdevno++;
	}
	
	printf("can't open ethertap device, aborting\n");
	return -1;
}


static int ethertap_down(void)
{
	int skfd;
	struct ifreq ifr;

    	/* Open a socket so we can ioctl() */
        if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket");
                exit(1);
        }
	
   	sprintf(ifr.ifr_name, "tap%d", tapdevno);

	
	/* Set the interface to DOWN */
	ifr.ifr_flags &= ~IFF_UP; /* Set it to ~UP (down ;) */
   	if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
	       	perror("SIOCSIFFLAGS");
        	return -1;
    	}

	close(skfd);		
	if (fd >= 0) 
		close(fd);

	return 0;
}


static int ethertap_read(char *frame,int length)
{
	return read(fd,frame,length);
}

static int ethertap_write(char *frame, int sz) 
{
	return write(fd,frame,sz);
}

static struct interface_t ethertap = {
	"ethertap",
	"$Id$",
	ethertap_setup,
	ethertap_down,
	ethertap_read,
	ethertap_write
};

/* Do Dynamic initialisation */
void _init(void) {
	register_device(&ethertap);
}
