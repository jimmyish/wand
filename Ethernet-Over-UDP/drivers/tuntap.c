/* 
 *  WANd Project - Ethernet Over UDP
 * 
 *  $Id$
 *
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */
  
  
/*
 *  tuntap.c - Tuntap driver for Etud
 *  
 *  James Spooner < james@spoons.gen.nz >
 *
 *  read/write/setup the tuntap linux device
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
#include <linux/if_tun.h>

#include "driver.h"
#include "debug.h"

static char tapdevname[32];
static int fd = -1;

static int tuntap_setup(char *req_name) {

       struct ifreq ifr;

	strcpy(tapdevname, req_name);

	logger(MOD_DRIVERS, 7, "tuntap_setup () entered...\n");
	
	fd = 0;

	fd = open("/dev/net/tun", O_RDWR);       


	if (fd > 1) {
	
		memset(&ifr, 0, sizeof(ifr));
		
		ifr.ifr_flags = IFF_TAP;
		snprintf(ifr.ifr_name, IFNAMSIZ, tapdevname);
  				            
		if( (ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
		      close(fd);
		      return -1;
		}
		
		logger(MOD_DRIVERS, 7, "Interface is: %s\n", ifr.ifr_name); 
		return fd;
	} else {
		logger(MOD_DRIVERS, 6, "/dev/net/tun could not be opened, does the file exist, and is tuntap support\n");
		logger(MOD_DRIVERS, 6, "included in the kernel?\n");
	}
	return 0;
}

static int tuntap_read(char *frame, int length)
{
  int c;
	char buf[length+2];

	c = read(fd, buf, length + 2);
	memcpy(frame, buf+2, c - 2);
	return c - 2;
}


static int tuntap_write(char *frame, int sz) 
{
        int c;
	char buf[sz+2];
  
	bzero(buf,sz+2);
	memcpy(buf+2,frame,sz);  
	c = write (fd, buf, sz + 2);
	return c - 2;
}

static int tuntap_down(void)
{
        int skfd;
        struct ifreq ifr;
        
        
        /* Open a socket so we can ioctl() */
        
        if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket");
                exit(1);
        }
         
        snprintf(ifr.ifr_name, IFNAMSIZ, tapdevname);

        /* Set the interface to DOWN */
        ifr.ifr_flags &= ~IFF_UP; /* Set it to ~UP (down ;) */
        if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
                perror("SIOCSIFFLAGS");
                return -1;
        }       
                
        close(skfd);
        
        if (fd > 0)
                close(fd);

	// return the old fd, so we can close properly
        return fd;
}

static struct interface_t tuntap = {
        "tuntap",
        "$Id$",
        tuntap_setup,
        tuntap_down, 
        tuntap_read, 
        tuntap_write 
};

/* Do Dynamic initialisation */
void _init(void) {
        register_device(&tuntap);
}
