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
 * 
 * Some helpfull (but damn hard to find and remember after a few months !)
 * man pages:
 * 
 * man 7 netdevice  - ioctl's and ifreq struct decscriptions for linux 
 *                    devices
 */
 
#include <sys/ioctl.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>

#include "ethertap_cfg.h" /* grab max and default /dev/tapX numbers */
#include "driver.h"
#include "debug.h"

static int fd = -1;
char *ifname;

static int ethertap_setup(char *req_name) 
{
	char tapdevice[16];
	int tapdevno = FIRST_TAP_NUMBER;
	int skfd;
	struct ifreq ifr;

    	/* Open a socket so we can ioctl() */
        if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                logger(MOD_DRIVERS, 1, "Socket create failed - %m\n");
                return -1;
        }
		
	logger(MOD_DRIVERS, 15, "ethertap_setup() entered...\n");


	assert(strlen(req_name) <= IFNAMSIZ);
	
	ifname = strdup(req_name);
	
	if(ifname == NULL) {
		logger(MOD_DRIVERS, 1, "ifname copy failed.\n");
		return -1;
	}
	
	while (tapdevno < MAX_ETHERTAP_DEVICES) {
		snprintf(tapdevice, 16, "/dev/tap%d", tapdevno);
		fd = open(tapdevice, O_RDWR);
		if(fd >= 0){
			logger(MOD_DRIVERS, 15, "got device %s (fd=%d)\n", tapdevice, fd);
			break;
		}
		tapdevno++;
	}
	
	if(fd < 0){
		logger(MOD_DRIVERS, 1, "Can't open ethertap device, aborting.\n");
		return -1;
	}
	
	snprintf(ifr.ifr_name, IFNAMSIZ, "tap%d", tapdevno);
	snprintf(ifr.ifr_newname, IFNAMSIZ, "%s", ifname);
	
	if(ioctl(skfd, SIOCSIFNAME, &ifr) < 0){
		logger(MOD_DRIVERS, 1, 
				"Could not rename ethertap interface to %s - %m.\n", 
				ifname);
		return -1;
	}

	logger(MOD_DRIVERS, 15, "Ethertap interface renamed to %s.\n", ifname);
	logger(MOD_DRIVERS, 15, "ethertap_setup() completed...\n");

	return fd;
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
	
   	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));

	
	/* Set the interface to DOWN */
	ifr.ifr_flags &= ~IFF_UP; /* Set it to ~UP (down ;) */
   	if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
	       	perror("SIOCSIFFLAGS");
        	return -1;
    	}

	close(skfd);		
	if (fd >= 0) 
		close(fd);

	return fd;
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
