#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
              
int main(int argc,char **argv)
{
	char buf[1024];
	int size;
	struct sockaddr_un sockname;
	int fd=socket(PF_UNIX,SOCK_STREAM,0);
	if (fd<0) {
		perror("control socket");
		return 1;
	}
	sockname.sun_family = AF_UNIX;
	strcpy(sockname.sun_path,"/var/run/Etud.ctrl");
	if (connect(fd,(const sockaddr *)&sockname,sizeof(sockname))<0) {
		perror("control connect");
		close(fd);
		return -1;
	}
	if (write(fd,argv[1],strlen(argv[1]))!=strlen(argv[1]) || write(fd,"\r\n",2)!=2)
		return 2;
	alarm(2);
	while ((size=read(fd,buf,1024))>1)
		write(1,buf,size);
}
