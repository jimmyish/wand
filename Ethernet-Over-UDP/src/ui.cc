/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

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
#include <errno.h>


#include "list.h"
#include "ui.h"
#include "udp.h"
#include "driver.h"
#include "mainloop.h"
#include "debug.h"
#include "etud.h"

/* "ADD mac ip [port]" */
static void m_add(int fd,char **argv,int argc)
{
	if (argc<2) {
		ui_send(fd,"-ERR Not enough parameters\r\n");
		return;
	}
	ether_t ether;
	if( 0 > ether.parse(argv[1]) ) {
		ui_send(fd,"-ERR MAC address does not grok\r\n");
		return;
	}
	ip_t ip;
	if ((signed int)(ip=inet_addr(argv[2]))==-1) {
		ui_send(fd,"-ERR IP address does not grok\r\n");
		return;
	}
	/* todo add port */
	if( add_ip(ether,ip) ) {
	  ui_send(fd,"-OK updated\r\n");
	} else {
	  ui_send(fd,"-OK no change\r\n");
	}
	return;
}

/* "DEL mac" */
static void m_del(int fd,char **argv,int argc)
{
	ether_t ether;
	if (argc<1) {
		ui_send(fd,"-ERR Too few arguments\r\n");
	}
	if( 0 > ether.parse(argv[1]) ) {
		ui_send(fd,"-ERR MAC address does not grok\r\n");
	}
	rem_ip(ether);
	ui_send(fd,"-OK deleted\r\n");
	return;
}

/* GETMAC */
static void m_getmac(int fd,char **argv,int argc)
{
	char tbuff[80];
	sprintf(tbuff,"+GETMAC %s\r\n", macaddr);
	ui_send(fd, tbuff);
	ui_send(fd, "-OK\r\n");

}

/* GETPORT */
static void m_getport(int fd,char **argv,int argc)
{
	char tbuff[80];
	sprintf(tbuff,"+GETPORT %u\r\n", udpport);
	ui_send(fd, tbuff);
	ui_send(fd, "-OK\r\n");

}

/* LIST */
static void m_list(int fd,char **argv,int argc)
{
	char tbuff[80];
	ui_send(fd,"+LIST ethernet\tip\r\n");
	for (online_t::const_iterator i=online.begin();
	     i!=online.end();
	     i++) 
	{
		struct sockaddr_in sockaddr;
		sockaddr.sin_addr.s_addr=i->second;
		sprintf(tbuff,"+LIST %s\t%s\r\n",
			i->first(),
			inet_ntoa(sockaddr.sin_addr));
		ui_send(fd,tbuff);
	}
	ui_send(fd,"-OK\r\n");
	return;
}

static void m_version(int fd,char **argv,int argc)
{
	ui_send(fd,"-ERR Not Supported\r\n");
	return;
}

struct functable_entry_t {
	char *name;
	void (*func)(int fd,char **,int);
}; 

static struct functable_entry_t functable[] = {
	{ "ADD", m_add },
	{ "DEL", m_del },
	{ "GETMAC", m_getmac },
	{ "GETPORT", m_getport },
	{ "LIST", m_list },
	{ "VERSION", m_version },
	{ NULL, NULL }
};

/* Parses a line */
static int parse(char *line,char **argv,int argc)
{
	int arg=0;
	argv[0] = line;
	while (*line && arg<argc) {
		if (*line == ' ') {
			*line='\0';
			arg++;
			argv[arg] = line+1;
		}
		line++;
	}
	return arg;
}

#define MAX_ARGS 5
void ui_process_callback(int fd)
{
	char buffer[1024];
	int len;
	char *argv[MAX_ARGS];
	functable_entry_t *functable_iter;

	len=read(fd,buffer,sizeof(buffer));
	if( len < 0 ) {
		logger(MOD_IPC, 5, "Read error in callback: %s\n",
				strerror(errno));
		return;
	} else if( len==0 ) {
		remRead(fd);
		close(fd);
		return;
	}
	*(buffer+len)='\0';

	int argc=parse(buffer,argv,MAX_ARGS);

	
	for (functable_iter = functable;functable_iter->name;functable_iter++) {
		if (strcasecmp(functable_iter->name,argv[0]) == 0) {
			functable_iter->func(fd,argv,argc);
			return;
		}
	}
	ui_send(fd,"-ERR Unknown command\r\n");
	return;
}

int internal_send( int sock, char *msg, int msglen )
{
	int retval = 0;
	if( 0 > (retval = write(sock,msg,msglen) ) ) {
		logger(MOD_IPC, 5, "Write error in send: %s\n",
				strerror(errno));
		return retval;
	}

	return retval;
}

int ui_send(int sock,char *msg)
{
	if( internal_send( sock, msg, strlen(msg) ) != (int)strlen(msg) )
		return -1;
#if 0
	if( internal_send( sock, "\r\n", 2 ) != 2 )
#if 1 /* Ignore errors from writing "\r\n" */
		return 0;
#else
		return -1;
#endif
#endif
		
	return 0;
}

static void ui_callback(int fd)
{
	int fd2=accept(fd,NULL,0);
	if (fd2>=0){
		addRead(fd2,ui_process_callback);
		logger(MOD_IPC, 15, "UI accept succeeded\n");

	}
}

int ui_setup(char *s)
{
	int fd=socket(PF_UNIX,SOCK_STREAM,0);
	if (fd<0) {
		logger(MOD_IPC, 1, "Error creating control socket: %s\n",
				strerror(errno));
		return -1;
	}
	struct sockaddr_un sockname;
	sockname.sun_family = AF_UNIX;
	strcpy(sockname.sun_path,s);
	if (bind(fd,(const sockaddr *)&sockname,sizeof(sockname))<0) {
		logger(MOD_IPC, 1, "Error binding to control socket: %s\n",
				strerror(errno));
		close(fd);
		return -1;
	}
	if (listen(fd,8)<0) {
		logger(MOD_IPC, 1, "Error listening to control socket: %s\n",
				strerror(errno));
		close(fd);
		return -1;
	}
	addRead(fd,ui_callback);
	logger(MOD_IPC, 7, "UI ready\n");
	return fd;
}
