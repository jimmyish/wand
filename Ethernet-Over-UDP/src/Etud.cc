#include <sys/time.h> /* for select */
#include <sys/types.h> /* for select */
#include <net/ethernet.h> /* for ether_header and ETH_ALEN */
#include <iostream.h> /* for cout */
#include <dlfcn.h> /* for dlopen */
#include <unistd.h> /* for select */

#include "list.h"
#include "driver.h"
#include "udp.h"
#include "ui.h"
#include "mainloop.h"

int load_module(char *s)
{
	if(!dlopen(s,RTLD_NOW)) {
		cout << "Error loading module '" << s << "': " << dlerror() << endl;
		return 0;
	}
	return 1;
}

int main(int arvc,char **argv)
{
	int myid = 1; /* ID number on the network */

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
	if (!init_interface(interface,1)) {
		cout << "Failed to initialise interface." << endl;
		cout << "Aborting..." << endl;
		return 1;
	}
	if (udp_start()<0) {
		cout << "Failed to create udp socket." << endl;
		cout << "Aborting..." << endl;
		interface->down();
		return 1;
	}
	if (ui_setup()<0) {
		cout << "Failed to create unix domain socket." << endl;
		interface->down();
		return 1;
	}
	cout << "Etud started" << endl;
	cout << "Using interface driver: " << interface->name << endl;
	cout << " version: " << interface->version << endl;
	mainloop();
}