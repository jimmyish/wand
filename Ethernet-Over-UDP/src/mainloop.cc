#include <sys/types.h>
#include <unistd.h>
#include <map>
#include "mainloop.h"

typedef map<int,callback_t> fd2callback_t;

static fd2callback_t fd2callback;

static fd_set rfd;
static int highestfd = 0;

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

	while(1) {
		rfd2=rfd;
		select(highestfd+1,&rfd2,NULL,NULL,NULL);
		for (fd2callback_t::const_iterator i=fd2callback.begin();
		     i!=fd2callback.end();
		     i++)
			if (FD_ISSET(i->first,&rfd2))
				i->second(i->first);
	}
}