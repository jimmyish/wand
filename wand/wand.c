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
#include "daemons.h"
#include "protoverlay.h"

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
	struct node_t *tmp = root;
	if (!root) {
		tmp = (struct node_t*)malloc(sizeof(struct node_t));
		root = tmp;
	}
	else {
		while (tmp->next) {
			if (strcasecmp(mac,tmp->mac)==0) {
				if (strcmp(ip,tmp->ip)!=0) {
					free(tmp->ip);
					tmp->ip=strdup(ip);
				}
				tmp->flag=0;
				return;
			}
			tmp=tmp->next;
		}
		tmp->next = (struct node_t*)malloc(sizeof(struct node_t));
		tmp=tmp->next;
	}
	// syslog("Adding %s (%i)",mac,ip)
	tmp->mac = strdup(mac);
	tmp->ip = strdup(ip);
	tmp->flag = 0;
	tmp->next = NULL;
}

/*
 * clearOldEntries
 *
 * Remove any entry from Etud and from our list if it's flag is 1, which
 * means that wand doesn't know about it either.
 */
void clearOldEntries(void)
{
	struct node_t *tmp = root;
	struct node_t *tmp2 = NULL;
	struct node_t *next = NULL;
	char message[255];
	while (tmp) {
		next = tmp->next;
		if (tmp->flag) {
			// syslog("Removing %s",mac)
    			snprintf(message,sizeof(message),
					"DEL %s %s",
					tmp->mac,tmp->ip);
    			tellEtud(message);
			if (!tmp2) {
				root = root->next;
				free(tmp->ip);
				free(tmp->mac);
				free(tmp);
			}
			else {
				tmp2->next = tmp->next;
				free(tmp->ip);
				free(tmp->mac);
				free(tmp);
			}
		}
		else {
			temp->flag=1; /* old */
			tmp2=tmp;
		}
		tmp=next;
	}
}

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
	struct hostent *host;

	if (argc<2) {
	  printf("%s serverip mac [controlfile]\n",argv[0]);
	  return 1;
	}

	if (argc>3) {
		strcpy(control_file_path,argv[3]);
	}
	else {
		strcpy(control_file_path,"/var/run/Etud.ctrl");
	}

	if (sock<0) {
		perror("socket");
		return 1;
	};

	srand(time(NULL));
	
	address.sin_family = AF_INET;
	address.sin_port = htons(0);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	host = gethostbyname(argv[1]);

	if (!host) {
		fprintf(stderr,"%s: Not found\n",argv[1]);
		return 1;
	}

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
	put_pid("wand");
	
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
		  sendto(sock,argv[2],strlen(argv[2])+1,0,
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
