#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../Ethernet-Over-UDP/include/daemons.h"

static int testspeed = 0;

#define BUFLEN 1024
void tellEtud(char *msg)
{
  struct sockaddr_un sockname;
  char buffer[BUFLEN] = {'\0'};
  int fd=socket(PF_UNIX,SOCK_STREAM,0);
  if (fd<0) { 
    perror("control socket");
    fprintf(stderr,"Didn't write '%s'",msg);
    return;
  } 
  
  sockname.sun_family = AF_UNIX;
  strcpy(sockname.sun_path,"/var/run/Etud.ctrl");
  
  if (connect(fd,(const struct sockaddr *)&sockname,sizeof(sockname))<0) { 
    perror("control connect(/var/run/Etud.ctrl)"); 
    fprintf(stderr,"Didn't write '%s'",msg);
    close(fd); 
    return;
  }
  
  if (write(fd,msg,strlen(msg))!=strlen(msg) || write(fd,"\r\n",2)!=2)
    return;
  
  read(fd,buffer,BUFLEN-1);
  buffer[BUFLEN-1] = '\0';
  /* We don't do anything with the buffer anyway. oh well */
  close(fd);
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

void doPacket(char *packet,int len)
{
  while(len>0) {
    char *mac=getword(&packet,&len);
    char *ip=getword(&packet,&len);
    char message[255];
    snprintf(message,sizeof(message),"ADD %s %s",mac,ip);
    tellEtud(message);
  }
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

	if (argc<2) {
	  printf("%s serverip mac\n",argv[0]);
	  return 1;
	}

	if (sock<0) {
		perror("socket");
		return 1;
	};

	if( argc >= 3 && 0 == strcmp(argv[2],"test" )) {
	  fprintf( stderr, "TestSpeed on, will update faster\n" );
	  testspeed++;
	}
	srand(time(NULL));
	
	address.sin_family = AF_INET;
	address.sin_port = htons(0);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(44444);
	if(!inet_aton(argv[1],&serveraddr.sin_addr)) {
	  	perror("inet_aton");
	}
	if (bind(sock,(struct sockaddr *)&address,sizeof(address))<0) {
		perror("bind");
		return 1;
	}
	
	daemonise();
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
		    if( testspeed == 0 ) {
		      tm.tv_sec=300+rand()%600;
		    } else {
		      tm.tv_sec=60+rand()%60;
		    }
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
