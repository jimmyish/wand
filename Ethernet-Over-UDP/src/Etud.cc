#include <sys/time.h> /* for select */
#include <sys/types.h> /* for select */
#include <net/ethernet.h> /* for ether_header and ETH_ALEN */
#include <iostream.h> /* for cout */
#include <dlfcn.h> /* for dlopen */
#include <unistd.h> /* for select */

#include "list.h"
#include "driver.h"
#include "udp.h"

int load_module(char *s)
{
	if(!dlopen(s,RTLD_NOW)) {
		cout << "Error loading module '" << s << "': " << dlerror() << endl;
		return 0;
	}
	return 1;
}

void process_packets(int udpfd,int interfacefd,struct interface_t *driver)
{
	while(1) {
		fd_set rfds;
		static char buffer[65536];
		FD_ZERO(&rfds);
		FD_SET(udpfd,&rfds);
		FD_SET(interfacefd,&rfds);
		select(((udpfd>interfacefd)?udpfd:interfacefd)+1,&rfds,NULL,NULL, NULL);
		if (FD_ISSET(udpfd,&rfds)) {
			int size=udp_read(udpfd,buffer,sizeof(buffer));
			if (driver->write(buffer,size)!=size) {
				cerr << "Failed writing device frame" << endl;
			}
		}
		if (FD_ISSET(interfacefd,&rfds)) {
			int size=driver->read(buffer,sizeof(buffer));
			ip_t *ip = find_ip((ether_t *)((ether_header *)(buffer))->ether_dhost);
			if (!ip) {
				cerr << "Failed to find destination host" << endl;
			}
			else if (write(udpfd,buffer,size)!=size) {
				cerr << "Failed writing udp packet" << endl;
			}
		}
	}
}

int main(int arvc,char **argv)
{
	int myid = 1; /* ID number on the network */
	int ifd = 0; /* Interface file descriptor */
	int ufd = 0; /* UDP file descriptor */

	if (!load_module("drivers/ethertap.so")) {
		cout << "Aborting..." << endl;
		return 1;
	}
	struct interface_t *interface = find_interface("ethertap");
	if ((interface=find_interface("ethertap"))==NULL) {
		cout << "Failed to find driver" << endl;
		cout << "Aborting..." << endl;
		return 1;
	}
	if ((ifd=interface->setup(1))<0) {
		cout << "Failed to initialise interface." << endl;
		cout << "Aborting..." << endl;
		return 1;
	}
	if ((ufd=udp_start())<0) {
		cout << "Failed to create udp socket." << endl;
		cout << "Aborting..." << endl;
		interface->down();
		return 1;
	}
	cout << "Etud started" << endl;
	cout << "Using udp port: " << 0 << endl;
	cout << "Using interface driver: " << interface->name << endl;
	cout << " version: " << interface->version << endl;
	process_packets(ufd,ifd,interface);
}