/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef DRIVER_H
#define DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

/* The following should be in <sys/ioctls.h> but it isn't.
 *
 * It is in <linux/sockios.h> but including that directly isn't a cool idea !
 *
 */
	
#define SIOCSIFNAME     0x8923          /* set interface name */

	
struct interface_t {
	char *name;
	char *version;
	int (*setup)(char *req_name);
	int (*down)(void);
	int (*read)(char *frame,int length);
	int (*write)(char *frame,int length);
};

extern struct interface_t *driver;

void register_device(struct interface_t *interface_description);
int init_interface(void);
int shutdown_interface(void);
void send_interface(char *buffer,int size);

#ifdef __cplusplus
}
#endif

#endif
