#include "driver.h"
#include <vector>

typedef vector<struct interface_t *> interface_list_t;

static interface_list_t drivers;


void add_device(struct interface_t *interface)
{
	drivers.push_back(interface);
	cout << "Registered device: " << interface->name << endl;
}

struct interface_t *find_interface(char *s)
{
	for (interface_list_t::const_iterator i=drivers.begin();
	     i!=drivers.end();
	     i++) {
		if (strcmp((*i)->name,s)==0)
			return *i;
	}
}


extern "C" {

  void register_device(struct interface_t *interface)
  {
	add_device(interface);
  }

}
