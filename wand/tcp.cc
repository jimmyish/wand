#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "tcp.hh"

host2host_t::host2host_t(char *hostname,int port=22222)
{
	struct hostent *host;
	int i=0;
	int flag=0;
	int fd;
	struct sockaddr_in sockname;

	fd=socket(PF_INET,SOCK_STREAM,0);
	if (fd<0) {
		return;
	}
	sockname.sin_family = AF_INET;
	sockname.sin_port = htons(port);
	host = gethostbyname(hostname);
	/* If we can't resolve the name, abort */
	if (!host) {
		close(fd);
		return;
	}
	/* Connect to each possible IP */
	for (i=0;host->h_addr_list[i];i++) {
		/* Parse the address, if failed, skip it, try the next */
		if (!inet_aton(host->h_addr_list[i],&sockname.sin_addr))
			continue;
		/* Try and connect, if failed, skip it, try the next */
		if (connect(fd,(const sockaddr *)&sockname,sizeof(sockname))<0)
			continue;
		flag=1;
		break;
	}
	if (!flag) {
		/* Failed to connect, give up */
		close(fd);
		return;
	}
	setFD(fd);
	return;
}
       
