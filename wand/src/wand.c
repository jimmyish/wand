#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <libgen.h> /* for basename */
#include <signal.h>
#include <dlfcn.h>
#include "daemons.h"
#include "protoverlay.h"
#include "config.h"
#include "debug.h"
#include "wand.h"


extern int modtolevel[];
extern int default_log_level;

char *control_file_path = 0;
int sock;
char macaddr[18];
unsigned short our_etud_port = 0;
struct sockaddr_in address;

char *getword(char **buffer,int *len)
{
  char *tmp=*buffer;
  while(**buffer && *len)
  {
    (*buffer)++;
    (*len)--;
  }
  (*buffer)++;
  (*len)--;
  return tmp;
}

int load_module(char *filename)
{
	if(!dlopen(filename,RTLD_NOW)) {
		logger(MOD_INIT, 1, "Error loading module '%s': %s\n", filename , dlerror());
		return 0;
	}
	
	return 1;
}


/* addEntry
 * 
 * adds/updates an entry in the mac->ip mapping linked list.  It always
 * sets flag to 0.
 */
void addEntry(char *mac,char *ip)
{
	struct node_t *curr = root;
	struct node_t *prev = NULL;

	while (curr) {
		if (strcasecmp(mac,curr->mac)==0) {
			if (strcmp(ip,curr->ip)!=0) {
				free(curr->ip);
				curr->ip=strdup(ip);
			}
			curr->flag=0;
			return;
		}
		prev = curr;	
		curr = curr->next;
	}
	
	if(prev == NULL){
		assert(root == NULL);
		root = (struct node_t*)malloc(sizeof(struct node_t));
		curr = root;
	} else {
		prev->next = (struct node_t*)malloc(sizeof(struct node_t));
		curr = prev->next;
	}
	
	// syslog("Adding %s (%i)",mac,ip)
	curr->mac = strdup(mac);
	curr->ip = strdup(ip);
	curr->flag = 0;
	curr->next = NULL;
}

void send_update(int s, siginfo_t *t, void *m)
{
	sendto(sock, macaddr, strlen(macaddr)+1, 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
}		  


/*
 * clearOldEntries
 *
 * Remove any entry from Etud and from our list if it's flag is 1, which
 * means that wand doesn't know about it either.
 */
void clearOldEntries(void)
{
	struct node_t *curr = root;
	struct node_t *prev = NULL;
	struct node_t *next = NULL;
	char message[255];
	while (curr) {
		next = curr->next;
		if (curr->flag) {
			// syslog("Removing %s",mac)
    			snprintf(message,sizeof(message),
					"DEL %s",
					curr->mac);
    			tellEtud(message, control_file_path);
			if (!prev) {
				assert(root == curr);
				root = root->next;
				free(curr->ip);
				free(curr->mac);
				free(curr);
			}
			else {
				prev->next = curr->next;
				free(curr->ip);
				free(curr->mac);
				free(curr);
			}
		}
		else {
			curr->flag=1; /* old */
			prev=curr;
		}
		curr=next;
	} /* of while(tmp) */
} /* of clearOldEntries */

void doPacket(char *packet,int len)
{
  while(len>0) {
    char *mac=getword(&packet,&len);
    char *ip=getword(&packet,&len);
    char message[255];
    addEntry(mac,ip);
    /* currently tell Etud that the remote port for this pair is the 
     * same as our port. FIXME */
    snprintf(message,sizeof(message),"ADD %s %s %d",mac,ip,our_etud_port);
    tellEtud(message, control_file_path);
  }
  clearOldEntries();
}

void usage(const char *prog) {
	char *progname;

        progname=strdup(prog);
        

	printf("%s:	[-c ctrlfile]	- Specify the Etud control file"
"	[-D]		- Don't daemonise"
"	[-f configfile]	- Read config from this file"
"	[-h]		- This help"
"	[-i server]	- Specify the wansd server "
"	[-l port]	- Communicate on the specified port"
"	[-L level]      - Default logging level, 0 = silent, 15 = noisy"
"	[-p pidfile]	- File to store pid in"
"	[-P protocol]	- The protocol module to use"
"Options on command line override those in the config file.				\n", basename(progname));
}

int mainloop(void)
{
	int flag = 0; /* If set it means we've sent an update request
			 packet, but not yet had a reply back. (therefore
			 resend the request every 30s or so, instead of
			 every 10 minutes
		      */
	struct timeval tm;
	int expire;

	tm.tv_sec=0;
	tm.tv_usec=0;
	for (;;) {
	  	int addrlen = sizeof(address);
		char buffer[65536];
		int len;
		int now;
		fd_set rfds;

		now = time(NULL);

		if (expire >= now) {
		  if (flag) {
	            expire = now + 30;
		  }
		  else 
		  {
	            expire = now + 300 + rand()%600;
		  }
		  flag=1;
		  sendto(sock,macaddr,strlen(macaddr)+1,0,
		    (struct sockaddr *)&serveraddr, sizeof(serveraddr));
		}
		FD_ZERO(&rfds);
		FD_SET(sock,&rfds);
		tm.tv_usec = 0;
		tm.tv_sec = expire - now;
		select(sock+1,&rfds, NULL, NULL, &tm);
		if (FD_ISSET(sock,&rfds)) {
			len=recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr *)&address,(socklen_t *)&addrlen);
			doPacket(buffer,len);
			flag=0;
		}
	};
	return 0;
}

/* Config options from the command line */
char *cpidfile=NULL;
char *cprotocol=NULL;
char *ccontrol_file_path=NULL;
int cdo_daemonise=-1;
char *conffile = 0;
struct hostent *host = NULL;
int cudpport=-1;
int clevel=-1;

int parse_commandline(int argc,char **argv)
{
	char ch;
	// Parse command line arguments
	while((ch = getopt(argc, argv, "c:Df:h:i:l:L:p:P:")) != -1){
	  switch(ch)
	    {	
			case 'c':
				ccontrol_file_path = strdup(optarg);
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
				host = gethostbyname(optarg);
				if (!host) {
					fprintf(stderr,"%s: Not found\n",optarg);
					return 1;
				}
				break;
			case 'l':
				cudpport = atoi(optarg);
				break;
			case 'L':
                                if(sscanf(optarg, "%i", &default_log_level) 
                                   == 0)
                                        default_log_level = -1;
                                clevel = default_log_level;
                                break;
			case 'p':
				cpidfile = strdup(optarg);
				break;
			case 'P':
				cprotocol = strdup(optarg);
				break;
			default:
				usage(argv[0]);
				return 1;
				break;
		}
	}
	return 0;
}

int main(int argc,char **argv)
{
	response_t *resp;

	/* The real config options */
	int do_daemonise=1;
	char *pidfile = "wand";
	char *server = 0;
	char *proto = 0;
	int udpport = 44444;
	
	/* Signal handling */
	struct sigaction usr1;
	
	/* Config options from the config file */
	config_t main_config[] = {
		{"server", TYPE_STR|TYPE_NULL, &server},
		{"controlfile", TYPE_STR|TYPE_NULL, &control_file_path},
		{"daemonise", TYPE_BOOL|TYPE_NULL, &do_daemonise},
		{"udpport", TYPE_INT|TYPE_NULL, &udpport},
		{"protocol", TYPE_STR|TYPE_NULL, &proto},
		{ "debug_default", TYPE_INT|TYPE_NULL, &default_log_level},
                { "debug_MOD_INIT", TYPE_INT|TYPE_NULL, &modtolevel[MOD_INIT]},
                { "debug_MOD_IPC", TYPE_INT|TYPE_NULL, &modtolevel[MOD_IPC]},
		{NULL, 0, NULL}
	};

	// Set defaults
	conffile = strdup("/usr/local/etc/wand.conf");
	control_file_path = strdup("/var/run/Etud.ctrl");

	parse_commandline(argc,argv);
	
	/* Set up the signal handler - bind SIGUSR1 to send an update instantly */
	usr1.sa_sigaction = send_update;
	usr1.sa_flags = SA_SIGINFO;
	sigaction(SIGUSR1, &usr1, 0);
	
	/* Check the default log level is sane. */
	if(default_log_level < 0 || default_log_level > 15) {
		default_log_level = 15;
		logger(MOD_INIT, 1, "Default logging level must be a number"
				" between 0 and 15. Giving up.\n");
		return 1;
	}
	
	/* Read the config file */
	if(parse_config(main_config, conffile))
	{
		logger( MOD_INIT, 3, "Error parsing config file: %s\n", conffile);
		return 1;
	}
	
	/* Override config file with command line options if set */
	if (!host && server) 
		host = gethostbyname(server);
	if (ccontrol_file_path != NULL)
		control_file_path = strdup(ccontrol_file_path);
	if (cudpport!=-1) 
		udpport = cudpport;
	if (cpidfile != NULL) 
		pidfile = strdup(cpidfile);
	if (clevel != -1)
		default_log_level=clevel;
	
	/* Check that required parameters are there */
	if(!host)
	{
		logger( MOD_INIT, 3, "Cannot resolve hostname\n");
		return 1;
	}

	sock = socket(PF_INET, SOCK_DGRAM, 0);
	
	if (sock<0) {
		perror("socket");
		return 1;
	};

	/* Get the MAC address from Etud */
	resp = askEtud("GETMAC", control_file_path);
	
	if( resp->status == OKAY) {
		strncpy(macaddr, resp->data[0]+7, 17);
		macaddr[17]='\0';	
	}

	else {
		logger(MOD_IPC, 1, "Could not retrieve MAC address from Etud - Exiting\n");
		return 1;
	}
	
	delete_response( resp );
	free( resp );
	
	resp = askEtud("GETPORT", control_file_path);
	
	if( resp->status == OKAY) {
		our_etud_port = atoi(resp->data[0]+8);
	}

	else {
		logger(MOD_IPC, 1, "Could not retrieve port from Etud - Exiting\n");
		return 1;
	}
	
	delete_response( resp );

	free( resp );

	/* check that the port we got from Etud is sane */

	if( our_etud_port <= 0 || our_etud_port > 65535 ){
		logger(MOD_IPC, 1, "Got a bad port from Etud - Exiting");
		return 1;
	}

	srand(time(NULL));
	
	address.sin_family = AF_INET;
	address.sin_port = htons(0);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(udpport);
	memcpy(&serveraddr.sin_addr.s_addr,
			host->h_addr,
			sizeof(serveraddr.sin_addr.s_addr));
	if (bind(sock,(struct sockaddr *)&address,sizeof(address))<0) {
		perror("bind");
		return 1;
	}
	
	daemonise(argv[0]);
	put_pid(pidfile);

	return mainloop();
}

