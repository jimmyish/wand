#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

void tellEtud(char *msg)
{
  int size;
  struct sockaddr_un sockname;
  int fd=socket(PF_UNIX,SOCK_STREAM,0);
  if (fd<0) { 
    perror("control socket");
    return;
 } 

 sockname.sun_family = AF_UNIX;
 strcpy(sockname.sun_path,"/var/run/Etud.ctrl");

 if (connect(fd,(const struct sockaddr *)&sockname,sizeof(sockname))<0) { 
   perror("control connect"); 
   close(fd); 
   return;
 }

 if (write(fd,msg,strlen(msg))!=strlen(msg) || write(fd,"\r\n",2)!=2)
   return;
 close(fd);
}

char *getword(char **buffer,int *len)
{
  char *tmp=*buffer;
  while(**buffer && *len)
  {
    *buffer++;
    *len--;
  }
  buffer++;
  return tmp;
}

int doPacket(char *packet,int len)
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
	struct sockaddr_in address;
	struct sockaddr_in serveraddr;
	int errors=0;

	if (sock<0) {
		perror("socket");
		return 1;
	};

	srand(time(NULL));
	
	address.sin_family = AF_INET;
	address.sin_port = htons(44440);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(44444);
	inet_aton(serveraddr.sin_addr,argv[1]);
	if (bind(sock,(struct sockaddr *)&address,sizeof(address))<0) {
		perror("bind");
		return 1;
	}
	for (;;) {
	  	int addrlen = sizeof(address);
		char buffer[65536];
		int len;
		sendto(sock,argv[2],strlen(argv[2])+1,0,
		    (struct sockaddr *)&serveraddr, sizeof(serveraddr));
		/* TODO: Do a select here */
		len=recvfrom(sock,buffer,sizeof(buffer),0,
		    (struct sockaddr *)&address,&addrlen);
		doPacket(buffer,len);
	  	sleep(rand()%600+300);
	};
	return 0;
}
