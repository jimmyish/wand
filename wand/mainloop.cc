#include "mainloop.hh"
#include <time.h>

mainloop_t::mainloop_t()
{
	running = false;
}

void mainloop_t::addEvent(time_t time,event_t event,void *data)
{
	eventnode_t *node = new eventnode_t;
	node->time=time;
	node->event=event;
	node->data=data;
	eventlist.push(node);
}

time_t mainloop_t::getNextEvent()
{
	time_t now=::time(NULL);
	while (!eventlist.empty() &&eventlist.top()->time<=now) {
		struct eventnode_t *node;
		node=eventlist.top();
		eventlist.pop();
		node->event(node->data);
		delete node;
		now=::time(NULL);
	}
	if (eventlist.empty())
		return -1;
	else
		return eventlist.top()->time-now;
}

