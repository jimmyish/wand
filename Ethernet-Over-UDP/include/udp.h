/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef UDP_H
#define UDP_H

extern int udpfd;
extern int udpport;

int udp_start();
int udp_read(int fd,char *buffer,int buffer_size);

#endif
