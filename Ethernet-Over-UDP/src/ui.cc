#include <sys/types.h> /* for socket */
#include <sys/socket.h> /* for socket */
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include "list.h"
#include "ui.h"
#include "mainloop.h"
#include "driver.h"

void ui_process_callback(int fd)
{
	char buffer[1024];
	int len;
	len=read(fd,buffer,sizeof(buffer));
	if (len==0) {
		remRead(fd);
		close(fd);
		return;
	}
	*(buffer+len)='\0';
	char *arg=buffer;
	while(*arg && *arg != ' ')
		arg++;
	if (*arg == ' ' || *arg == '\n' || *arg == '\r') {
		*arg='\0';
		arg++;
	}
	if (strcasecmp("add",buffer) == 0) {
		if (!arg) {
			ui_send(fd,"ERR Missing parameter ether");
			return;
		}
		char *arg2;
		arg2=arg;
		while (*arg2 && *arg2 != ' ')
			arg2++;
		if (*arg2 == '\0') {
			ui_send(fd,"ERR Missing parameter ip");
			return;
		}
		*arg2='\0';
		arg2++;
		ether_t ether;
		ether.parse(arg);
		ip_t ip;
		if ((ip=inet_addr(arg2))==-1) {
			ui_send(fd,"ERR IP address parse error");
			return;
		}
		add_ip(ether,ip);
		return;
	}
	else if (strcasecmp("del",buffer) == 0) {
		ether_t ether;
		ether.parse(arg);
		rem_ip(ether);
		return;
	}
	else if (strcasecmp("list",buffer) == 0) {
		char buffer[80];
		ui_send(fd,"LIST- ethernet\tip");
		for (online_t::const_iterator i=online.begin();
		     i!=online.end();
		     i++) 
		{
			struct sockaddr_in sockaddr;
			sockaddr.sin_addr.s_addr=i->second;
			sprintf(buffer,"LIST- %s\t%s",
				i->first(),
				inet_ntoa(sockaddr.sin_addr));
			ui_send(fd,buffer);
		}
		ui_send(fd,"LIST");
		return;
	}
	else if (strcasecmp("version",buffer) == 0) {
		ui_send(fd,"ERR Not Supported");
		return;
	}
	else if (strcasecmp("",buffer)!=0) {
		ui_send(fd,"ERR Invalid or unsupported command");
		return;
	}
	return;
}

int ui_send(int sock,char *msg)
{
	if (write(sock,msg,strlen(msg))!=(int)strlen(msg))
		return -1;
	return (write(sock,"\r\n",2)==2) ? 0 : -1;
}

static void ui_callback(int fd)
{
	int fd2=accept(fd,NULL,0);
	if (fd2>=0)
		addRead(fd2,ui_process_callback);
}

int ui_setup(char *s="/var/run/Etud.ctrl")
{
	int fd=socket(PF_UNIX,SOCK_STREAM,0);
	if (fd<0) {
		perror("control socket");
		return -1;
	}
	struct sockaddr_un sockname;
	sockname.sun_family = AF_UNIX;
	strcpy(sockname.sun_path,s);
	if (bind(fd,(const sockaddr *)&sockname,sizeof(sockname))<0) {
		perror("control bind");
		close(fd);
		return -1;
	}
	if (listen(fd,8)<0) {
		perror("control listen");
		close(fd);
		return -1;
	}
	addRead(fd,ui_callback);
	printf("UI ready\n");
	return fd;
}
