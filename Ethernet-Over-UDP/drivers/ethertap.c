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
 
#define MAX_ETHERTAP_DEVICES 16
#define FCS 2

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

#include "driver.h"

static int fd = -1;
static int tapdevno;

static int ethertap_setup(unsigned long myid) {
	
	struct ifreq ifr;
	int skfd;
	char tapdevice[16];

	char hwaddr[6] = { 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	fd = 0;	
	tapdevno = 2;

	
	memcpy(hwaddr, &(myid)+(6-sizeof(myid)), 4);
	
    	/* Open a socket so we can ioctl() */
        if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket");
                exit(1);
        }

               	
	while (tapdevno<MAX_ETHERTAP_DEVICES) {
		snprintf(tapdevice, 255, "/dev/tap%d", tapdevno);
		fd = open(tapdevice, O_RDWR);					

		if (fd > 1) {

   			/* Get interface flags */
   			sprintf(ifr.ifr_name, "tap%d", tapdevno);

  	  		if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
   	     			fprintf(stderr, "%s: unknown interface: %s\n", 
                		ifr.ifr_name, strerror(errno));
        			return -1;
    			}

			/* Set the interface to DOWN */
			ifr.ifr_flags &= ~IFF_UP; /* Interfaces don't like their HWADDR changed on the fly! */
		   	if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
			       	perror("SIOCSIFFLAGS");
       	 			return -1;
    			}
#if 0			
			/* Set the HWADDR */
			ifr.ifr_netmask.sa_family = ARPHRD_ETHER;

			memcpy(ifr.ifr_netmask.sa_data, hwaddr, 6);
    			if (ioctl(skfd, SIOCSIFHWADDR, &ifr) < 0) {
            			perror("SIOCSIFHWADDR");
        			return -1;
    			}
#endif    			
			/* MTU is 576 */
			ifr.ifr_mtu = 576;
            		if (ioctl(skfd, SIOCSIFMTU, &ifr) < 0) {
                            	fprintf(stderr, "SIOCSIFMTU: %s\n", strerror(errno));
				return -1;
                        }
                                                        
			close (skfd);
			fprintf(stderr,"ethertap bound to device /dev/tap%i\n",tapdevno);
			return fd;
		}			
		tapdevno++;
	}


	close (skfd);
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