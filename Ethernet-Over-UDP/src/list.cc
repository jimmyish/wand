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


#ifdef INSANE_LIST_DEBUG
//#warning Fun!
#define DPRINTF(x...) fprintf( stderr, x )
#define DUMP() dump_table( stderr )
//#warning Fun^2!
FILE *debfile = NULL;
#define DPRINTF2(x...) { \
  debfile = fopen("debug.log", "at"); \
  fprintf( debfile, x ); \
  fclose( debfile ); }
#define DUMP2() { \
  debfile = fopen("debug.log", "at"); \
  dump_table( debfile ); \
  fclose( debfile ); }
//#warning Fun_over!
#else
#define DPRINTF(x...)  
#define DUMP()
#endif

online_t online;

bool add_ip(ether_t ether,ip_t ip)
{
  bool retval = true;

  if( !online.empty() ) {
    online_t::iterator i = online.begin();
    ip_t found = false;
    DPRINTF( "\nadd_ip(%s, %x) Entered\n", ether(), ip );
    DUMP();
    while( i != online.end() ) { /* Not empty, search for given ether */
      if( ((*i).first) == ether ) { /* match, handle */
	found = (*i).second;
	if( found == ip ) { /* ether and ip both match - no change */
	  DPRINTF( "\nadd_ip() Unchanged (will return f)\n" );
	  return false;
	} else { /* node has changed, update it */
	  (*i).second = ip;
	  DPRINTF( "\nadd_ip() Changed (will return f)\n" );
	  DUMP();
	  return false;
	}
      }
      ++i;
    }
    /* given ether is not in list, add it */
    DPRINTF( "\nadd_ip() Was Not In List (will return T)\n" );
    online.push_back( node_t::pair(ether,ip) );
    DUMP();
    return true;
  } else { /* List was empty, add the node */
    DPRINTF( "\nadd_ip(%s, %x) Was Empty (will return T)\n", ether(), ip );
    online.push_back( node_t::pair(ether,ip) );
    DUMP();
    return true;
  }
  /* should never get here */
  return retval;
}

bool rem_ip(ether_t ether)
{
  if (online.empty() ) {
    DPRINTF( "rem_ip() Empty\n" );
    /* No need to dump the table -- it's empty :) */
    return false;
  }
  DPRINTF("\nrem_ip(%s) Entered\n", ether() );
  DUMP();

  online_t::iterator i = online.begin();
  bool found_and_removed = false;
  while( i != online.end() ) {
    if( ((*i).first) == ether ) { /* match, remove */
      found_and_removed = true;
      online.remove( *i );
      break;
    }
    ++i;
  }
  DPRINTF( "rem_ip() = %s\n", (found_and_removed)?"TRUE":"FALSE");
  DUMP();
  return found_and_removed;
}

ip_t find_ip(ether_t ether)
{
  if (online.empty() ) {
    DPRINTF( "find_ip() Empty\n" );
    /* No need to dump the table -- it's empty :) */
    return false;
  }
  DPRINTF("\nfind_ip(%s) Entered\n", ether() );
  DUMP();

  online_t::iterator i = online.begin();
  ip_t found = false;
  
  while( i != online.end() ) {
    if( ((*i).first) == ether ) { /* match, return */
      found = (*i).second;
      break;
    }
    ++i;
  }
  DPRINTF( "find_ip() = %x\n", found );
  DUMP();
  return found;
}

int dump_table( FILE *stream )
{
  if( online.empty() ) {
    fprintf( stream, "Table is empty.\n" );
    return 0;
  }
  online_t::iterator i = online.begin();
  int c = 0;
  while( i != online.end() ) {
    fprintf( stream, "%2i: online[\"%s\"] = %x\n", c,
	     ((*i).first)(), (*i).second );
    ++i;
    ++c;
  }
  fprintf( stream, "--\n" );
  return c;
}


#ifdef TEST
#include <assert.h>

int main(int argc,char **argv)
{
	ether_t ether;
	ether_t ether2;
	ether.parse("01:02:03:04:05:06");
	ether2.parse("01:02:03:04:05:07");
	assert(rem_ip(ether)==false);
	add_ip(ether,ip_t(0x04030201));
	assert(find_ip(ether) == ip_t(0x04030201));
	assert(rem_ip(ether)!=false);
	assert(rem_ip(ether)==false);
	add_ip(ether,ip_t(0x04030201));
	add_ip(ether,ip_t(0x04030201));
	add_ip(ether2,ip_t(0x04030202));
	assert(find_ip(ether2) == ip_t(0x04030202));
	assert(find_ip(ether) == ip_t(0x04030201));
	assert(rem_ip(ether)!=false);
	assert(rem_ip(ether2)!=false);
}
#endif

