/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <sys/stat.h> /* for umask */
#include <unistd.h> /* for getpid, write, close, fork, setsid, chdir */
#include <fcntl.h> /* for creat,open */
#include <stdio.h> /* for snprintf */

#include "daemons.h"

/* Flag set if we are a daemon or not. If we are then set to one. Used
 * to tell if we should send output to screen or syslog.
 */

int daemonised = 0;

void put_pid( char *fname )
{
	char *defname = "WandProject";
	char buf[512];

	if( fname == NULL ) fname = defname;

	snprintf( buf, 512, "/var/run/%s.pid", fname );
	int fd=creat(buf,0660);
	if (fd<0)
		return;
	sprintf(buf,"%i\n",getpid());
	if (write(fd,buf,strlen(buf)) != (signed)strlen(buf)) {
		close(fd);
		return;
	}
	close(fd);
}

void daemonise( void ) 
{
	switch (fork()) {
	case 0:
		break;
	case -1:
		perror("fork");
		exit(1);
	default:
		_exit(0);
	}
	setsid();
	switch (fork()) {
       	case 0:
       		break;
       	case -1:
       		perror("fork2");
		exit(1);
	default:
		_exit(0);
	}
	chdir("/");
	umask(0155);
	close(0);
	close(1);
	close(2);
	open("/dev/null",O_RDONLY);
	open("/dev/console",O_WRONLY);
	open("/dev/console",O_WRONLY);
}	
