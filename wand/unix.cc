#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include "unix.hh"

unix_t::unix_t(char *filename)
{
	struct sockaddr_un sockname;
	int fd;
	fd=socket(PF_UNIX,SOCK_STREAM,0);
	if (fd<0) {
		return;
	}
	sockname.sun_family = AF_UNIX;
	strcpy(sockname.sun_path,filename);
	if (connect(fd,(const sockaddr *)&sockname,sizeof(sockname))<0) {
		close(fd);
		return;
	}
}
