/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

/* <wuz> Do we need sys/time.h, sys/types.h, sys/stat.h ? */
#include <sys/time.h> /* for select */
#include <sys/types.h> /* for select, umask and open */
#include <sys/stat.h> /* for umask and open */
#include <net/ethernet.h> /* for ether_header and ETH_ALEN */
#include <dlfcn.h> /* for dlopen */
#include <unistd.h> /* for select */
#include <fcntl.h> /* for open */
#include <getopt.h> /* for parsing command line options */
#include <syslog.h>

#include "list.h"
#include "driver.h"
#include "udp.h"
#include "ui.h"
#include "mainloop.h"
#include "daemons.h"
#include "debug.h"
#include "config.h"

extern int modtolevel[];
char *macaddr=NULL;
char *ifname="wan0";

int load_module(char *filename)
{
	if(!dlopen(filename,RTLD_NOW)) {
		logger(MOD_INIT, 1, "Error loading module '%s': %s\n",
				filename , dlerror());
		
		return 0;
	}
	return 1;
}

void usage(const char *prog) {
	printf("%s: [-d module] 		 - Transport driver to use
							[-D]						 - Don't daemonise
							[-f configfile]	 - Read config from this file
							[-h]						 - This help
							[-i ifname]			 - Name of the interface to create 
							[-l port]				 - Communicate on the specified port
							[-m macaddr]		 - MAC address for the created interface
							[-p pidfile] 		 - File to store pid in
							Options on command line override those in the config
							file.					
							\n", prog);

}

int main(int argc,char **argv)
{
	/* Actual configuration options */
	int do_daemonise=1;
	char *module=NULL;
	char *conffile=NULL;
	char *pidfile="Etud";
	
	/* Temporary options read from the command line */
	char *cmacaddr=NULL;
	char *cmodule=NULL;
	char *cpidfile=NULL;
	int cdo_daemonise=1;
	int cudpport=-1;
	
	/* Possible config file options */
	config_t main_config[] = {
		{ "module", TYPE_STR|TYPE_NOTNULL, &module },
		{ "daemonise", TYPE_BOOL|TYPE_NULL, &do_daemonise },
		{ "macaddr", TYPE_STR|TYPE_NULL, &macaddr },
		{ "ifname", TYPE_STR|TYPE_NULL, &ifname },
		{ "pidfile", TYPE_STR|TYPE_NULL, &pidfile },
		{ "udpport", TYPE_INT|TYPE_NULL, &udpport },
		{ "debug_MOD_INIT", TYPE_INT|TYPE_NULL, &modtolevel[MOD_INIT]},
		{ "debug_MOD_IPC", TYPE_INT|TYPE_NULL, &modtolevel[MOD_IPC]},
		{ "debug_MOD_DRIVERS", TYPE_INT|TYPE_NULL, &modtolevel[MOD_DRIVERS]},
		{ NULL, 0, NULL }
	};

	// Parse command line arguments
	char ch;
	while((ch = getopt(argc, argv, "dDf:hi:l:m:p:")) != -1){
	switch(ch){	
			case 'd':
				cmodule = strdup(optarg);
				break;
			case 'D':
				cdo_daemonise=0;
				break;
			case 'f':
				conffile = strdup(optarg);
				break;
			case 'h':
				usage(argv[0]);
				return 0;
				break;
			case 'i':
				ifname = strdup(optarg);
				break;
			case 'l':
				cudpport = atoi(optarg);
				break;
			case 'm':
				cmacaddr = strdup(optarg);
				break;
			case 'p':
				cpidfile = strdup(optarg);
				break;
		}
	}

	/* Parse the config file */
	if (conffile != NULL) {
	  	logger(MOD_INIT, 15, "Parsing config file specified on command line\n");
	  	if (parse_config(main_config,conffile)) {
	    		logger(MOD_INIT,1,"Bad Config file %s, giving up\n", conffile);
	    		return 1;
	  	}
	} else {
		logger(MOD_INIT, 15, "About to parse default config file\n");
		if (parse_config(main_config,"/usr/local/etc/etud.conf")) {
			logger(MOD_INIT,1,"Bad Config file, giving up\n");
			return 1;
	  	}
	}
	
	/* Override the config file with any values specified on command line */
	if (cmacaddr != NULL)
		macaddr = strdup(cmacaddr);
	if (cmodule != NULL)
		module = strdup(cmodule);
	if (cpidfile != NULL) 
		pidfile = strdup(cpidfile);
	if (cdo_daemonise == 0)
		do_daemonise = 0;
	if (cudpport != -1)
		udpport = cudpport;
		
	/* Check that a MAC address has been specified */
	if (macaddr == NULL) {
		logger(MOD_INIT, 1, "No MAC Address specified!\n");
		return 1;
	}
	
	logger(MOD_INIT, 15, "Parsed config, about to load driver\n");
	if (!load_module(module)) {
		logger(MOD_INIT, 1, "Aborting...\n");
		return 1;
	}
	
	logger(MOD_INIT, 15, "Loaded driver, about to init interface\n");
	if (!init_interface()) {
		logger(MOD_INIT, 1, "Failed to initialise interface.\n");
		logger(MOD_INIT, 1, "Aborting...\n");
		return 1;
	}

	logger(MOD_INIT, 15, "Initialised interface, about to start UDP\n");
	if (udp_start()<0) {
		logger(MOD_INIT, 1, "Failed to create udp socket.\n");
		logger(MOD_INIT, 1, "Aborting...\n");
		shutdown_interface();
		return 1;
	}

	logger(MOD_INIT, 15, "UDP started, about to start UNIX domain socket\n");
	if (ui_setup()<0) {
		logger(MOD_INIT, 1, "Failed to create unix domain socket.\n");
		shutdown_interface();
		return 1;
	}

	logger(MOD_INIT, 6, "Etud started\n");
	logger(MOD_INIT, 6, "Using interface driver: %s\n", driver->name);
	logger(MOD_INIT, 6, " version: %s\n", driver->version);

	if (do_daemonise) {
		/* Lets go to the background */
		logger(MOD_INIT, 7, "Attempting to Daemonise...\n");

		daemonise(argv[0]);
		put_pid(pidfile);
		logger(MOD_INIT, 7, "Daemonised\n");
	}

	logger(MOD_INIT, 8, "Init complete\n");
	mainloop();
 	// Clean up the control file
	unlink("/var/run/Etud.ctrl");
      	// Clean up the pid file
      	if(daemonised){
            unlink(pidfile);
      	}
      	// shutdown the interface
	logger(MOD_INIT, 8, "Attempting to shutdown interface\n");
	shutdown_interface();
}
