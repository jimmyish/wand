#ifndef FD_HH
#define FD_HH
#include <unistd.h>
#include <iostream.h>

class filedescriptor_t {
	private:
		int	fd;
	public:
		filedescriptor_t(int fd2) { fd=fd2; };
		int getFD() const { return fd; }
		int ok() { return fd!=-1; }
		virtual void doRead(void) = 0;
		virtual void doWrite(void) = 0;
		virtual void doException(void) = 0;
		virtual ~filedescriptor_t() { std::cerr<<"bye"<<std::endl;close(fd); }
	protected:
		filedescriptor_t() : fd(-1) {};
		void setFD(int fd2) { fd=fd2; };
		int write(void *buffer,int bufflen) { return ::write(fd,buffer,bufflen); };
		int write(char *buffer) { return write(buffer,strlen(buffer)); }
		int read(void *buffer,int bufflen) { return ::read(fd,buffer,bufflen); };
};


#endif