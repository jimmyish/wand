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
#include <iostream.h> /* for cout */
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

int load_module(char *filename)
{
	if(!dlopen(filename,RTLD_NOW)) {
		logger(MOD_INIT, 1, "Error loading module '%s': %s\n",
				filename , dlerror());
		
		return 0;
	}
	return 1;
}

int main(int argc,char **argv)
{
	int do_daemonise=1;
	char *module=NULL;
	char *conffile=NULL;
	char *pidfile="Etud";
	config_t main_config[] = {
		{ "module", TYPE_STR|TYPE_NOTNULL, &module },
		{ "daemonise", TYPE_BOOL|TYPE_NULL, &do_daemonise },
		{ "debug_MOD_INIT", TYPE_INT|TYPE_NULL, &modtolevel[MOD_INIT]},
		{ "debug_MOD_IPC", TYPE_INT|TYPE_NULL, &modtolevel[MOD_IPC]},
		{ "debug_MOD_DRIVERS", TYPE_INT|TYPE_NULL, &modtolevel[MOD_DRIVERS]},
		{ NULL, 0, NULL }
	};

	// Parse command line arguments
	char ch;
	while((ch = getopt(argc, argv, "f:p:")) != -1){
	  switch(ch)
	    {	
	    case 'f':
	      conffile = strdup(optarg);
				break;
			case 'p':
				pidfile = strdup(optarg);
				break;
	    }
	}
	
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
		//device->down();
		return 1;
	}

	logger(MOD_INIT, 15, "UDP started, about to start UNIX domain socket\n");
	if (ui_setup()<0) {
		logger(MOD_INIT, 1, "Failed to create unix domain socket.\n");
		//interface->down();
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
      	if (do_daemonise) {
            unlink(pidfile);
      	}
      	// shutdown the interface
	//syslog(LOG_DAEMON,"Attempting to shutdown interfacce\n");
	//logger(MOD_INIT, 8, "Attempting to shutdown interface\n");
	//shutdown_interface();
}
