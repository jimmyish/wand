/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <sys/types.h>
#include <unistd.h>
#include <signal.h> /* for sigaction (call and struct) */
#include <stdio.h> /* for fprintf and perror */
#include <map>
#include "mainloop.h"

typedef map<int,callback_t> fd2callback_t;

static fd2callback_t fd2callback;

static fd_set rfd;
static int highestfd = 0;


/* Set SIGPIPE to SIG_IGN - if wand goes AWOL we don't really want to die 
 * return >=0 on sucess
 */
int add_sig_hnd( void )
{
	struct sigaction handler;
  
	/* Assign our_sig_hnd as our signal handler */
	handler.sa_handler = SIG_IGN;
  
	/* We don't want to block any other signals while handling this one */
	sigemptyset(&handler.sa_mask);

	/* Continously ignore this */
	handler.sa_flags = SA_RESTART;

	/* Make these values effective. */
	if (sigaction(SIGPIPE, &handler, NULL) < 0) {
		perror("add_sig_hnd:sigaction");
		return -1;
	}

	return 0;
}

void addRead(int fd,callback_t callback)
{
	FD_SET(fd,&rfd);
	fd2callback[fd]=callback;
	if (highestfd<fd)
		highestfd=fd;
}

void remRead(int fd)
{
	FD_CLR(fd,&rfd);
}

void mainloop(void)
{
	fd_set rfd2;
  	
	if( 0 > add_sig_hnd() ) {
		fprintf( stderr, "Unable to add Signal Handler - "
			 "Not Catching Signals!\n" );
	}
	while(1) {
		rfd2=rfd;
		select(highestfd+2,&rfd2,NULL,NULL,NULL);
		for (fd2callback_t::const_iterator i=fd2callback.begin();
		     i!=fd2callback.end();
		     i++)
			if (FD_ISSET(i->first,&rfd2))
				i->second(i->first);
	}
}
