#ifndef FDSERVER_HH
#define FDSERVER_HH
#include "fd.hh"
#include "mainloop.hh"
#include <assert.h>
#include <sys/socket.h>
#include <stdio.h>

template <class server_t>
class sockserver_t : public filedescriptor_t {
	public:
		virtual void doRead(void) {
			int fd=::accept(getFD(),NULL,0);
			if (fd<0) {
				perror("accept");
				printf("%i\n",getFD());
			}
			mainloop->addFD(new server_t(fd));
			mainloop->addFD(this);
		};

		virtual void doWrite(void) { assert(0); };
		virtual void doException(void) { assert(0); }
};

#endif