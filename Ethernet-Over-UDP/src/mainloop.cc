/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <sys/types.h>
#include <sys/time.h> /* for timeval, select timeouts */
#include <unistd.h>
#include <signal.h> /* for sigaction (call and struct) */
#include <stdio.h> /* for fprintf and perror */
#include <map>
#include <errno.h>
#include "debug.h"
#include "mainloop.h"

typedef map<int,callback_t> fd2callback_t;

static fd2callback_t fd2callback;

static fd_set rfd;
static int highestfd = 0;

volatile int endloop=0;

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
		logger(MOD_INIT, 3, "Failed to add signal handler:"
			" SIGPIPE, %s\n", strerror(errno));
		return -1;
	}
	
	/* Add a handler to SIGTERM */
	handler.sa_handler = &sig_hnd;
	sigemptyset(&handler.sa_mask);
	if (sigaction(SIGTERM, &handler, NULL) < 0) {
	  	logger(MOD_INIT, 3 , "Failed to add signal handler:"
			" SIGTERM, %s\n", strerror(errno));
	  	return -1;
	}

	return 0;
}

void sig_hnd( int signo ) 
{
  logger(MOD_IPC, 6, "Caught Signal - Ending\n");
  // Set the end flag
  endloop = 1;

}

void addRead(int fd,callback_t callback)
{
	assert(fd!=-1);
	FD_SET(fd,&rfd);
	fd2callback[fd]=callback;
	if (highestfd<fd)
		highestfd=fd;
	logger(MOD_IPC, 15, "Added fd %d, highest fd = %d\n", fd, highestfd);
}

void remRead(int fd)
{
	FD_CLR(fd,&rfd);
}

void mainloop(void)
{
	fd_set rfd2;
	struct timeval timeout;

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	
	if( 0 > add_sig_hnd() ) {
		logger(MOD_INIT, 4, "Unable to add Signal Handler - "
			"Not Catching Signals!\n" );
	}
	while(!endloop) {
	  rfd2=rfd;
	  select(highestfd+2, &rfd2, NULL, NULL, &timeout);
	  for (fd2callback_t::const_iterator i=fd2callback.begin(); 
	       i!=fd2callback.end(); 
	       i++) {
	    
	    if (FD_ISSET(i->first,&rfd2)) {
	      i->second(i->first);
	    }
	    if (endloop) {
	      break;
	    }
	  }

	}
	// Close file descriptors
	for (fd2callback_t::const_iterator i=fd2callback.begin(); 
		     i!=fd2callback.end(); 
		     i++)
	close(i->first);
}
