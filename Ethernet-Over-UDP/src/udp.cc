/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <assert.h>
#include <netinet/in.h> /* for AF_INET and sockaddr_in */
#include <unistd.h> /* for read() */
#include <errno.h>
#include <string.h>

#include "driver.h"
#include "mainloop.h"
#include "udp.h"
#include "debug.h"

int udpfd;
int udpport;

int udp_read(int udpfd,char *buffer,int size)
{
	return read(udpfd,buffer,size);
}

const int BUFFERSIZE=65536;

static void udp_callback(int fd)
{
	static char buffer[BUFFERSIZE];
	int size;
	size=udp_read(udpfd,buffer,BUFFERSIZE);
	if (size<16)
		return;
	send_interface(buffer,size);
};

int udp_start(void)
{
	struct sockaddr_in addr;
	int sock;

	if ((sock = socket(AF_INET,SOCK_DGRAM,0))<0) {
		logger(MOD_NETWORK, 1, "Failed to create network socket: %s\n",
				strerror(errno));
		return -1;
	}

	addr.sin_family=AF_INET;
	addr.sin_port=htons(udpport);
	addr.sin_addr.s_addr=htonl(INADDR_ANY);

	if ((bind(sock,(struct sockaddr *)&addr,sizeof(addr)))<0) {
		logger(MOD_NETWORK, 1, "Failed to bind to network socket: %s\n",
				strerror(errno));
		return -1;
	}
	udpfd=sock;
	addRead(sock,udp_callback);
	return sock;
}
