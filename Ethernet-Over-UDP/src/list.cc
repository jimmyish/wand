/*
 * This file maintains a list of who is connected to the network, their
 * "ethernet" address and their "ip" address.
 */

#include <map>
#include <vector>
#include <algorithm>
#include "list.h"

struct ether_less {
	bool operator ()(ether_t *a,ether_t *b) const {
		for (int i=0;i<6;i++)
			if ((*a)[i] < (*b)[i])
				return true;
			else if ((*a)[i] > (*b)[i])
				return false;
		return false;
	}
};

/* Maps an ethernet address to 0,1, or more IP's. */
typedef std::map<ether_t *, std::vector < ip_t * >, ether_less > online_t;

online_t online;

void add_ip(ether_t *ether,ip_t *ip)
{
	online[ether].push_back(ip);
}

bool rem_ip(ether_t *ether,ip_t *ip)
{
	online[ether].erase(find(online[ether].begin(),online[ether].end(),ip));
}

ip_t *find_ip(ether_t *ether)
{
	if (online[ether].size()==0)
		return NULL;
	else
		return online[ether][online[ether].size()];
}