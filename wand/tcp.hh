#ifndef TCP_HH
#define TCP_HH
#include "fd.hh"
#include "fdserver.hh"
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>


class host2host_t : public filedescriptor_t {
	public:
		host2host_t(char *filename,int port);
};

template <class server_t>
class tcpserver_t : public sockserver_t<server_t> {
	public:
		tcpserver_t(int port)
                {	
                	struct sockaddr_in sockname;
                	int servfd=socket(PF_INET,SOCK_STREAM,0);
                	sockname.sin_family = AF_INET;
                	sockname.sin_port = htons(port);
                	sockname.sin_addr.s_addr = htonl(INADDR_ANY);
			int on=1;
			if (setsockopt(servfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on))<0) {
				perror("setsockopt(REUSEADDR)");
			}
                	if (bind(servfd,(const struct sockaddr *)&sockname,sizeof(sockname))<0) {
				perror("bind");
                		close(servfd);
                	}
                	if (listen(servfd,0)<0) {
				perror("listen");
                		close(servfd);
                	}
                	setFD(servfd);
                	return;
                }

};

#endif