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
#include "driver.h"
#include "mainloop.h"
#include "debug.h"

void ui_process_callback(int fd)
{
	char buffer[1024];
	int len;
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
	char *arg=buffer;
	while( *arg && !isspace( *arg ) )
		arg++;
	while( *arg && isspace( *arg ) ) {
		*arg='\0';
		arg++;
	}

	if( arg >= (buffer+len)) 
		logger(MOD_IPC, 15, "GOT HERE! arg: past EOS buf: \"%s\"\n", 
				buffer);
	else
		logger(MOD_IPC, 15, "GOT HERE! arg: \"%s\" buf: \"%s\"\n", 
				arg, buffer);


	/* "ADD 00:01:02:03:04:05 1.2.3.4" */
	if (strcasecmp("add",buffer) == 0) {
		if (!arg) {
			ui_send(fd,"-ERR Missing parameter 'mac'\r\n");
			return;
		}
		char *arg2;
		arg2=arg;
		while (*arg2 && *arg2 != ' ')
			arg2++;
		if (*arg2 == '\0') {
			ui_send(fd,"-ERR Missing parameter 'ip'\r\n");
			return;
		}
		*arg2='\0';
		arg2++;
		ether_t ether;
		if( 0 > ether.parse(arg) ) {
			ui_send(fd,"-ERR MAC address does not grok\r\n");
		}
		ip_t ip;
		if ((signed int)(ip=inet_addr(arg2))==-1) {
			ui_send(fd,"-ERR IP address does not grok\r\n");
			return;
		}
		if( add_ip(ether,ip) ) {
		  ui_send(fd,"-OK updated\r\n");
		} else {
		  ui_send(fd,"-OK no change\r\n");
		}
		return;
	}

	/* "DEL 00:01:02:03:04:05" */
	else if (strcasecmp("del",buffer) == 0) {
		ether_t ether;
		if( 0 > ether.parse(arg) ) {
			ui_send(fd,"-ERR MAC address does not grok\r\n");
		}
		rem_ip(ether);
		ui_send(fd,"-OK deleted\r\n");
		return;
	}

	/* "LIST" */
	else if (strcasecmp("list",buffer) == 0) {
		char buffer[80];
		ui_send(fd,"+LIST ethernet\tip\r\n");
		for (online_t::const_iterator i=online.begin();
		     i!=online.end();
		     i++) 
		{
			struct sockaddr_in sockaddr;
			sockaddr.sin_addr.s_addr=i->second;
			sprintf(buffer,"+LIST %s\t%s\r\n",
				i->first(),
				inet_ntoa(sockaddr.sin_addr));
			ui_send(fd,buffer);
		}
		ui_send(fd,"-OK\r\n");
		return;
	}
	
	/* "VERSION" (Not Yet Implemented) */
	else if (strcasecmp("version",buffer) == 0) {
		ui_send(fd,"-ERR Not Supported\r\n");
		return;
	}

	/* Non blank line - complain */
	else if (strcasecmp("",buffer)!=0) {
		ui_send(fd,"-ERR Invalid or unsupported command\r\n");
		return;
	}

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

int ui_setup(char *s="/var/run/Etud.ctrl")
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
