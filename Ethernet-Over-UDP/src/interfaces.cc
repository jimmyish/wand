#include "driver.h"
#include "list.h"
#include "udp.h"
#include "mainloop.h"
#include <net/ethernet.h>
#include <netinet/in.h>
#include <vector>
#include <map>
#include <sys/socket.h>

struct ether_header_t {
	unsigned short int fcs;
        struct ether_header eth;
};

typedef vector<struct interface_t *> interface_list_t;

static interface_list_t drivers;


void add_device(struct interface_t *interface)
{
	drivers.push_back(interface);
	cout << "Registered device: " << interface->name << endl;
}

struct interface_t *find_interface(char *s)
{
	for (interface_list_t::const_iterator i=drivers.begin();
	     i!=drivers.end();
	     i++) {
		if (strcmp((*i)->name,s)==0)
			return *i;
	}
	return NULL;
}


extern "C" {

  void register_device(struct interface_t *interface)
  {
	add_device(interface);
  }

}

typedef map<int,struct interface_t *> fd2interface_t;

static fd2interface_t fd2interface;

const int BUFFERSIZE = 65536;

static void udp_sendto(ip_t ip,char *buffer,int size)
{
	struct sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_port=htons(UDPPORT);
	dest.sin_addr.s_addr=ip;
	sendto(udpfd,buffer,size,0,(const struct sockaddr *)&dest,sizeof(dest));
}

static void do_read(int fd)
{
	static char buffer[BUFFERSIZE];
	int size=fd2interface[fd]->read(buffer,BUFFERSIZE);
	if (size<16) {
		printf("Runt, Oink! Oink! %i<16\n",size);
		return;
	};
	ether_t ether(((ether_header_t *)(buffer))->eth.ether_dhost);
	if (ether.isBroadcast()) {
		printf("Broadcasting %s\n",ether());
		for (online_t::const_iterator i=online.begin();
		     i!=online.end();
		     i++) 
			udp_sendto(i->second,buffer,size);
	}
	else {
		ip_t ip = find_ip(ether);
		if (ip) {
			printf("Sending %s to %i\n",ether(),ip);
			udp_sendto(ip,buffer,size);
		}
		else {
			printf("No match for %s\n",ether());
		};
	};
}

int init_interface(interface_t *interface)
{
	int ifd;
	if ((ifd=interface->setup(1))<0) {
		return 0;
	}
	addRead(ifd,do_read);
	fd2interface[ifd]=interface;
	return 1;
}

void send_interface(char *buffer,int size)
{
	fd2interface.begin()->second->write(buffer,size);
};