#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include "daemons.h"
#include "protoverlay.h"
#include "config.h"

char control_file_path[1024];

void tellEtud(char *msg)
{
  struct sockaddr_un sockname;
  struct timeval timeout;
  int fd=socket(PF_UNIX,SOCK_STREAM,0);
  response_t *resp = NULL;

  if (fd<0) { 
    perror("control socket");
    fprintf(stderr,"Didn't write '%s'",msg);
    return;
  } 
  
  sockname.sun_family = AF_UNIX;
  strcpy(sockname.sun_path,control_file_path);
  
  if (connect(fd,(const struct sockaddr *)&sockname,sizeof(sockname))<0) { 
    perror("control connect()"); 
    fprintf(stderr,"Didn't write '%s'",msg);
    close(fd); 
    return;
  }
  
  if (write(fd,msg,strlen(msg))!=strlen(msg) || write(fd,"\r\n",2)!=2)
    return;

  /* two second timeout between packets today */
  timeout.tv_usec = 0;
  timeout.tv_sec = 2;

  if( NULL != (resp = get_response( fd, &timeout )) ) {

    /* We don't do anything with the response anyway. oh well */
    delete_response( resp );
    free( resp );
  }
  close( fd );
}

response_t *askEtud(char *msg)
{
  struct sockaddr_un sockname;
  struct timeval timeout;
  int fd=socket(PF_UNIX,SOCK_STREAM,0);
  response_t *resp = NULL;

  if (fd<0) { 
    perror("control socket");
    fprintf(stderr,"Didn't write '%s'",msg);
    return NULL;
  } 
  
  sockname.sun_family = AF_UNIX;
  strcpy(sockname.sun_path,control_file_path);
  
  if (connect(fd,(const struct sockaddr *)&sockname,sizeof(sockname))<0) { 
    perror("control connect()"); 
    fprintf(stderr,"Didn't write '%s'",msg);
    close(fd); 
    return NULL;
  }
  
  if (write(fd,msg,strlen(msg))!=strlen(msg) || write(fd,"\r\n",2)!=2)
    return NULL;

  /* two second timeout between packets today */
  timeout.tv_usec = 0;
  timeout.tv_sec = 2;

  if( NULL == (resp = get_response( fd, &timeout )) ) {
    close( fd );
    return NULL;
  }

	close( fd );

	return resp;

}

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

struct node_t {
	char *mac;
	char *ip;
	int flag; /* Flag
		   * 0 -- Seen in this packet, don't remove it
		   * 1 -- Not yet seen, remove it on the next pass
		   */
	struct node_t *next;
} *root;

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
    			tellEtud(message);
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
    snprintf(message,sizeof(message),"ADD %s %s",mac,ip);
    tellEtud(message);
  }
  clearOldEntries();
}

void usage(const char *prog) {
	printf("%s: -i server 
					[-c controlfile] - Specify the Etud control file
					[-D]						 - Don't daemonise
					[-f configfile]	 - Read config from this file
					[-h]						 - This help
					[-i server]			 - Specify the wansd server 
					[-l port]				 - Communicate on the specified port
					[-p pidfile] 		 - File to store pid in
					Options on command line override those in the config
					file.					
					\n", prog);
}

int main(int argc,char **argv)
{
	int sock = socket(PF_INET, SOCK_DGRAM, 0);
	int flag = 0; /* If set it means we've sent an update request
			 packet, but not yet had a reply back. (therefore
			 resend the request every 30s or so, instead of
			 every 10 minutes
		      */
	struct sockaddr_in address;
	struct sockaddr_in serveraddr;
	struct timeval tm;
	struct hostent *host = NULL;
	response_t *resp;
	char *pidfile = "wand";
	char macaddr[18];
	char server[255];
	char ch;
	
	char conffile[1024];
	int do_daemonise=1;
	int cdo_daemonise=-1;
	
	config_t main_config[] = {
		{"server", TYPE_STR|TYPE_NOTNULL, &server},
		{"macaddr", TYPE_STR|TYPE_NOTNULL, &macaddr},	
		{"controlfile", TYPE_STR|TYPE_NOTNULL, &control_file_path},
		{"daemonise", TYPE_STR|TYPE_NOTNULL, &do_daemonise},
		{NULL, 0, NULL}
	};

	// Set defaults
	strcpy(conffile, "/usr/local/etc/wand.conf");
	strcpy(control_file_path,"/var/run/Etud.ctrl");
	macaddr[0] = '\0';
		
	// Parse command line arguments
	while((ch = getopt(argc, argv, "c:Df:h:i:l:p:")) != -1){
	  switch(ch)
	    {	
			case 'c':
				strncpy(control_file_path, optarg, 1024);
				break;
			case 'D':
				cdo_daemonise=0;
				break;
			case 'f':
				strncpy(conffile, optarg, 1024);
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
				
			case 'p':
				pidfile = strdup(optarg);
				break;
		}
	}

	if(parse_config(main_config, conffile))
		return 1;
	
	host = gethostbyname(server);

	if(!host)
		return 1;
	
	if (sock<0) {
		perror("socket");
		return 1;
	};

	/* Get the MAC address from Etud */
	resp = askEtud("GETMAC");
	print_response(resp, stdout);
	if( resp->status == OKAY) {
		strncpy(macaddr, resp->data[0]+7, 17);
	}
	delete_response( resp );
  free( resp );

	srand(time(NULL));
	
	address.sin_family = AF_INET;
	address.sin_port = htons(0);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(44444);
	memcpy(&serveraddr.sin_addr.s_addr,
			host->h_addr,
			sizeof(serveraddr.sin_addr.s_addr));
	if (bind(sock,(struct sockaddr *)&address,sizeof(address))<0) {
		perror("bind");
		return 1;
	}
	
	daemonise(argv[0]);
	put_pid(pidfile);
	
	tm.tv_sec=0;
	tm.tv_usec=0;
	for (;;) {
	  int addrlen = sizeof(address);
		char buffer[65536];
		int len;
		fd_set rfds;
		tm.tv_usec = 0;
		if (tm.tv_sec<1) {
		  if (flag) {
		    tm.tv_sec=30;
		  }
		  else 
		  {
		    tm.tv_sec=300+rand()%600;
		  }
		  flag=1;
		  sendto(sock,macaddr,strlen(macaddr)+1,0,
		    (struct sockaddr *)&serveraddr, sizeof(serveraddr));
		}
		FD_ZERO(&rfds);
		FD_SET(sock,&rfds);
		select(sock+1,&rfds, NULL, NULL, &tm);
		if (FD_ISSET(sock,&rfds)) {
			len=recvfrom(sock,buffer,sizeof(buffer),0,
		    	(struct sockaddr *)&address,&addrlen);
			doPacket(buffer,len);
			flag=0;
		}
	};
	return 0;
}
