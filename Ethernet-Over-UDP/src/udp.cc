#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <netinet/in.h> /* for AF_INET and sockaddr_in */
#include <unistd.h> /* for read() */

#include "udp.h"

int udp_start(int port=22222)
{
	struct sockaddr_in addr;
	int sock;

	if ((sock = socket(AF_INET,SOCK_DGRAM,0))<0) {
		perror("socket");
		return -1;
	}

	addr.sin_family=AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);

	if ((bind(sock,(struct sockaddr *)&addr,sizeof(addr)))<0) {
		perror("bind");
		return -1;
	}

	return sock;
}

int udp_read(int udpfd,char *buffer,int size)
{
	return read(udpfd,buffer,size);
}