#include <assert.h>
#include "select.hh"
#include <iostream>
#include <stdio.h>

select_t::select_t()
{
	FD_ZERO(&rfd);
	FD_ZERO(&wfd);
	FD_ZERO(&xfd);
	maxfd=0;
}

#define MAX(a,b) ((a<b) ? (b) : (a))

int select_t::addFD(filedescriptor_t *fd)
{
	std::cerr << "Adding fd " << fd->getFD() << std::endl;
	assert(fd->getFD()>=0 && fd->getFD()<FD_SETSIZE);
	fds[fd->getFD()]=fd;
	FD_SET(fd->getFD(),&rfd);
	FD_SET(fd->getFD(),&wfd);
	FD_SET(fd->getFD(),&xfd);
	maxfd=MAX(maxfd,fd->getFD()+1);
	return 0; /* No problems */
}

int select_t::remFD(filedescriptor_t *fd)
{
	std::cerr << "Removing fd " << fd->getFD() << std::endl;
	fds[fd->getFD()]=NULL;
	FD_CLR(fd->getFD(),&rfd);
	FD_CLR(fd->getFD(),&wfd);
	FD_CLR(fd->getFD(),&xfd);
	return -1;
}

int select_t::go()
{
	mainloop_t::go();
	assert(fds[4]->getFD()==4);
	while(isRunning()) {
		struct timeval timeout;

		fd_set trfd,twfd,txfd;
		trfd=rfd;
		twfd=wfd;
		txfd=xfd;
		time_t nextevent = getNextEvent();
		int count;
		if (nextevent>0) {
			timeout.tv_sec = nextevent;
			timeout.tv_usec = 0;
			count=select(maxfd,&trfd,&twfd,&txfd,&timeout);
		} else {
			count=select(maxfd,&trfd,&twfd,&txfd,NULL);
		}
		if (count<0) {
			perror("select");
			assert(count<0);
		}
 		for (int i=0;i<maxfd;i++) {
			assert(!FD_ISSET(i,&rfd) || i==fds[i]->getFD());
			//if (FD_ISSET(i,&rfd)) {
			//	std::cerr << i << ":" << fds[i]->getFD() << std::endl;
			//}

			if(FD_ISSET(i,&trfd)) {
				FD_CLR(i,&rfd);
				std::cerr << "Read ready on " << i << std::endl;
				fds[i]->doRead();
			}
			if(FD_ISSET(i,&twfd)) {
				FD_CLR(i,&wfd);
				std::cerr << "Write ready on " << i << std::endl;
				fds[i]->doWrite();
			}
			if(FD_ISSET(i,&txfd)) {
				FD_CLR(i,&xfd);
				fds[i]->doException();
			}
		} /* of for(i in descriptor sets) */
	} /* of while(running) */
	return 0; /* Ok */
} /* of select_t::go() */

mainloop_t *mainloop;

int main(int argc,char **argv)
{
	mainloop=new select_t();
	int error;
	error=init(argc,argv);
	if (!error)
		return mainloop->go();
	else
		return error;
}