#include <iostream>
#include "tcp.hh"
#include "unix.hh"
#include "select.hh"
#include "list.hh"
#include <netinet/in.h>
#include <arpa/inet.h>

online_t online;

class wand_t : public filedescriptor_t{
		int authenticated;
		char buffer[1024];
		char *bp;
	public:
		wand_t(int fd) : filedescriptor_t(fd) { 
			authenticated=0; 
			bp=buffer;
		};
		virtual void doRead() {
			int count=read(bp,1024-(bp-buffer));
			int dead=0;
			if (count!=0) {
				while(count>0) {
					if (*bp == '\n' || *bp=='\r' || *bp=='\0') {
						*bp='\0';
						if ((dead=doLine(buffer)))
							break;
						memmove(buffer,bp+1,count);
						bp=buffer;
					}
					else {
						bp++;
					}
					count--;
				}
				if (!dead)				
					mainloop->addFD(this);
			}
			else 
				dead=true;
			if (dead)
				delete this;
		}
		virtual void doWrite() {
		}
		virtual void doException() {
		}
		virtual ~wand_t() {
			mainloop->remFD(this);
		}
		int doLine(char *line) {
			std::cout<<"Got Line: " << line << std::endl;
			if (!authenticated) {
				if (strcmp(line,"password")) {
					write("Wrong password\n\r");
					return true;
				}
				authenticated=true;
				burstStatus();
				return false;
			}
			else {
				if (!strncasecmp(line,4,"add ")) {
					ip_t ip;
					
				}
			}
			return false;
		}
		void burstStatus() {
			for(online_t::const_iterator i=online.begin();
				i!=online.end();
				i++) {
				write("add ");
				write(i->first());
				write(" ");
				struct sockaddr_in sockname;
				sockname.sin_addr.s_addr=i->second;
				write(inet_ntoa(sockname.sin_addr));
				write("\n\r");
			}	
		}
};

int init(int argc,char **argv)
{
	tcpserver_t<wand_t> *tcpserver=new tcpserver_t<wand_t>(22222);
	mainloop->addFD(tcpserver);
	return 0;
}