/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include "driver.h"
#include "list.h"
#include "udp.h"
#include "mainloop.h"
#include <net/ethernet.h>
#include <netinet/in.h>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/if_arp.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "debug.h"
#include "etud.h"

struct ether_header_t {
	unsigned short int fcs;
        struct ether_header eth;
};

struct interface_t *driver;

extern "C" {

  void register_device(struct interface_t *interface)
  {
	driver = interface;
  }

}

const int BUFFERSIZE = 65536;

static void udp_sendto(ip_t ip,char *buffer,int size)
{
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_port=htons(udpport);
	dest.sin_addr.s_addr=ip;
	sendto(udpfd,buffer,size,0,(const struct sockaddr *)&dest,sizeof(dest));
}

static void do_read(int fd)
{
	static char buffer[BUFFERSIZE];
	int size=driver->read(buffer,BUFFERSIZE);
	if (size<16) {
		logger(MOD_IF,4,"Runt, Oink! Oink! %i<16\n",size);
		return;
	};
	ether_t ether(((ether_header_t *)(buffer))->eth.ether_dhost);
	if (ether.isBroadcast()) {
		logger(MOD_IF,15,"Broadcasting %s\n",ether());
		for (online_t::const_iterator i=online.begin();
		     i!=online.end();
		     i++) 
			udp_sendto(i->second,buffer,size);
	}
	else {
		ip_t ip = find_ip(ether);
		if (ip) {
			logger(MOD_IF,15,"Sending %s to %i\n",ether(),ip);
			udp_sendto(ip,buffer,size);
		}
		else {
			logger(MOD_IF,10,"No match for %s\n",ether());
		};
	};
}

int init_interface(void)
{

	struct ifreq ifr;
	int skfd;
	ether_t ether;

	/* Open a socket so we can ioctl() */
  if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
  	logger(MOD_IF, 1, "Socket create failed - %m\n");
    return -1;
  }

	int ifd;
	if ((ifd=driver->setup(ifname))<=0) {
		return 0;
	}
	
	/* Configure the MAC Address on the interface */
	snprintf(ifr.ifr_name, IFNAMSIZ, "%s", ifname);
	ether.parse(macaddr);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	memcpy(&ifr.ifr_hwaddr.sa_data, ether.address, sizeof(ether.address));
	if(ioctl(skfd, SIOCSIFHWADDR, &ifr) < 0) {
		logger(MOD_IF, 1, "Socket Set MAC Address failed - %m\n");
		return -1;
	}
	
	close(skfd);
	
	addRead(ifd,do_read);
	return 1;
}

int shutdown_interface(void)
{
	int ifd;
	if ((ifd=driver->down())<=0) {
		return 0;
	}
	remRead(ifd);
	return 1;
}
void send_interface(char *buffer,int size)
{
	driver->write(buffer,size);
}
