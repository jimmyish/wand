/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <stdio.h> /* printf */
#include <syslog.h> /* sylog */
#include <stdlib.h> /* free */
#include <stdarg.h> /* va */
#include <assert.h> /* assert */

#include "debug.h"

static int loglookup[]={LOG_ALERT, LOG_ALERT, LOG_CRIT, LOG_ERR, LOG_WARNING,
		 LOG_NOTICE, LOG_INFO, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG,
		 LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG, LOG_DEBUG,
		 LOG_DEBUG };

/* This defines the level at which each module is requesting be printed.
 * If the level passed to logger is higher than the level specified in this
 * array then we do not print the message.
 *
 * Initialise to zero so no messages are printed. The configuration system 
 * really really wants to set these higher !
 */

int modtolevel[]= {15, 15, 15, 15, 15, 15, 7};

extern int daemonised;

/* See debug.h for a description of the module and level definations. */

void logger(int module, int level, const char *format, ...)
{
	va_list ap;
	char buffer[513];
	
	/* Sanity checks. If these fail it's the person who is calling us
	 * at fault !
	 */
	
	assert(level <= 15 && level >= 0);
	assert(module <= 6 && module >= 0);
	assert(daemonised == 0 || daemonised == 1);
	assert(modtolevel[module] <= 15 && modtolevel[module] >= 0);
	
	va_start(ap, format);

	if(level <= modtolevel[module]){
		if(! daemonised){
			if(level < 7){
				vfprintf(stderr, format, ap);
			} else {
				vprintf(format, ap);
			}
		} else {
			vsnprintf(buffer, sizeof(buffer), format, ap);
			syslog(loglookup[level], "%s", buffer);
		}
	}

	va_end(ap);
}
	
