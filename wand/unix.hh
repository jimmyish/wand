#ifndef UNIX_HH
#define UNIX_HH
#include "fd.hh"
#include "fdserver.hh"

class unix_t : public filedescriptor_t {
	public:
		unix_t(char *filename);
};

template <class server_t>
class unixserver_t : public sockserver_t<server_t> {
	public:
		unixserver_t(char *filename) {};
};

#endif