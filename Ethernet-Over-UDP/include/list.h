#ifndef LIST_H
#define LIST_H

typedef char ether_t[6];
typedef char ip_t[4]; /* FIXME */

void add_ip(ether_t *ether,ip_t *ip);
bool rem_ip(ether_t *ether,ip_t *ip); /* false if not found */
ip_t *find_ip(ether_t *ether); /* return randomly an ip associated with this
				* interface id 
				*/

#endif

