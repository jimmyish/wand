#ifndef SELECT_HH
#define SELECT_HH
#ifdef USE_HASHMAP
#include <hash_map>
#define MAP hash_map
#else
#include <map>
#define MAP map
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include "fd.hh"
#include "mainloop.hh"

class select_t : public mainloop_t {
	private:
		fd_set rfd;
		fd_set wfd,xfd;
		int maxfd;
		std::MAP<int,filedescriptor_t *> fds;
	public:
		select_t();
		virtual int addFD(filedescriptor_t *fd);
		virtual int remFD(filedescriptor_t *fd);
		virtual int go();
		virtual ~select_t() {}; /* keep gcc happy */
};

#endif