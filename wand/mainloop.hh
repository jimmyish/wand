#ifndef MAINLOOP_HH
#define MAINLOOP_HH
#include <queue>
#include <time.h>
#include "fd.hh"

typedef void (*event_t)(void *);

//operator <(eventnode_t *a,eventnode_t *b) const { return a->time < b->time; };
class mainloop_t {
	private:
		int running;
		class eventnode_t {
			public:
				time_t time;
				event_t event;
				void *data;
		};
		std::priority_queue<eventnode_t *> eventlist;
	public:
		mainloop_t();
		virtual int addFD(filedescriptor_t *fd) = 0;
		virtual int remFD(filedescriptor_t *fd) = 0;
		void addEvent(time_t time,event_t event,void *data);
		time_t getNextEvent();
		int isRunning() const { return running; }
		void stop() { running = false; };
		virtual int go() { running = true; return 0;}
		virtual ~mainloop_t() {};
};

int init(int argc,char **argv);
extern mainloop_t *mainloop;

#endif