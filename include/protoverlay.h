/* Wand Project - Ethernet Over UDP
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#ifndef PROTOVERLAY_H
#define PROTOVERLAY_H
    
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
  
typedef enum {
  OKAY = 0, ERROR, MALFORMED,  TIMEOUT_NOTHING, TIMEOUT_DATA
} status_t;


typedef struct {
  status_t status;
  int count;
  char **data;
  
} response_t;

  
/* attempts to read packets from the given fd, with the given timeout
 * between packets.
 * completely allocates the response struct and returns that.
 * Writes to syslog a lot if anything goes wrong. May exit or return.
 */
response_t *get_response( int fd, struct timeval *timeout );

  
/* Removes a response cleanly and sanely.
 * if the response itself was malloc'd, follow this function with a
 * free on the response struct passed. (get_response meets this criteria)
 */
void delete_response( response_t *response );

  
/* outputs the given response on the given file stream in a nice pretty
 * format. Returns number of characters output.
 */
int print_response( response_t *response, FILE *stream );

/* Functions for communicating with Etud */
void tellEtud(char *msg, char *control_file_path);
response_t *askEtud(char *msg, char *control_file_path);

#ifdef __cplusplus
}
#endif

#endif /* PROTOVERLAY_H */
