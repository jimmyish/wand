/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

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
	online[ether]=ip;
}

bool rem_ip(ether_t ether)
{
	if (online.find(ether)==online.end())
		return false;
	online.erase(ether);
	return true;
}

ip_t find_ip(ether_t ether)
{
	if (online.find(ether)==online.end())
		return false;
	return online[ether];
}

#ifdef TEST
#include <assert.h>

int main(int argc,char **argv)
{
	ether_t ether;
	ether_t ether2;
	ether.parse("01:02:03:04:05:06");
	ether.parse("01:02:03:04:05:07");
	assert(rem_ip(ether)==false);
	add_ip(ether,ip_t(0x04030201));
	assert(find_ip(ether) == ip_t(0x04030201));
	assert(rem_ip(ether)!=false);
	assert(rem_ip(ether)==false);
	add_ip(ether,ip_t(0x04030201));
	add_ip(ether2,ip_t(0x04030202));
	assert(find_ip(ether) == ip_t(0x04030201));
	assert(find_ip(ether2) == ip_t(0x04030202));
	assert(rem_ip(ether)!=false);
	assert(rem_ip(ether2)!=false);
}
#endif

