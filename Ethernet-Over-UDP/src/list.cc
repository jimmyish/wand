/*
 * This file maintains a list of who is connected to the network, their
 * "ethernet" address and their "ip" address.
 */

#include <map>
#include <vector>
#include <algorithm>
#include "list.h"
#include <string>
#include <list>

online_t online;

void add_ip(ether_t ether,ip_t ip)
{
	if (online[ether]!=0)
		rem_ip(ether);
	online[ether]=ip;
}

bool rem_ip(ether_t ether)
{
	if (online[ether]==0)
		return false; /* TODO: Log error */
	return true;
}

ip_t find_ip(ether_t ether)
{
	return online[ether];
}

