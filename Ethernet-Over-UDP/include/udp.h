/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef UDP_H
#define UDP_H

#define UDPPORT 22222

extern int udpfd;

int udp_start(int port=UDPPORT);
int udp_read(int fd,char *buffer,int buffer_size);

#endif