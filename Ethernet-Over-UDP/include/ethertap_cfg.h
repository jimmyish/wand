/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */
        
#ifndef ETHERTAP_CFG_H
#define ETHERTAP_CFG_H
            
#define MAX_ETHERTAP_DEVICES 16
#define FCS 2

/* Default to /dev/tap0 */
/* <wuz> This doesn't work for me, I use /dev/tap3 */
#define FIRST_TAP_NUMBER 0
//#define FIRST_TAP_NUMBER 3

#endif /* ETHERTAP_CFG_H */
