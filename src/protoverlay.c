/* Wand Project - Protocol Overlay
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <sys/types.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
#include "protoverlay.h"


/* Wrapper around select() and read() 
 * select on the given fd for the given timeout period; if the timeout
 * expires, return zero. otherwise, returns the number of bytes
 * written to the given buffer, not more than bufsize. If an error occurs,
 * either terminate execution or return negative.
 * yes, it is possible to read 0 bytes, which will get mistaken for a
 * timeout. Suggestions Welcome.
 */
int select_on( int fd, char *buffer, int bufsize, struct timeval *timeout )
{
  struct timeval *to = NULL;
  fd_set readfds;
  int retval = 0;
  
  if( !buffer || bufsize <= 0 )
    return -2; /* EStupid */
  
  /* select() tends to alter timeout, so make a copy of it */
  if( timeout != NULL ) {
    if(NULL==( to = (struct timeval *)malloc( sizeof( struct timeval ) ) )) {
      fprintf( stderr, "Out of Memory in select_on!\n" );
      exit( 1 );
    }
    memcpy( to, timeout, sizeof( struct timeval ) );
  }
  
  FD_ZERO( &readfds );
  FD_SET( fd, &readfds );
  
  if( 0 > (retval = select( fd+1, &readfds, NULL, NULL, to ) ) ) {
    perror("select_on: select");
    exit( 1 );
  }
  
  if( to != NULL ) free( to );
  if( 0 == retval )
    return 0;
  
  if( !FD_ISSET( fd, &readfds ) ) {
    printf( "fd not set true. Err... I don't want to know what "
	    "went wrong.\n" );
    return -3;
  }
  /* We need our Terminating Null, so grab one less byte than needed
   */
  if( 0 > (retval = read( fd, buffer, bufsize-1 )) ) {
    perror("select_on: read");
    exit( 1 );
  }
  /* array contains elements in 0 :: retval-1, so set \0 on retval
   */
  buffer[ retval ] = '\0';
#if 0
  printf( "select_on(%i, %p, %i, %p) = %i *%s*\n", fd, buffer,
	  bufsize, timeout, retval+1, buffer );
#endif
  return retval+1;
}


int add_line( response_t *resp, char *buffer )
{
  int retval = 0;
  char *p = NULL;
  if( resp == NULL ) {
    syslog( LOG_DAEMON | LOG_WARNING, "add_line called with resp=NULL. "
	    "[%s:%u]\n", __FILE__, __LINE__ );
    return 0;
  }
  resp->count++;
  resp->data = realloc( resp->data, resp->count*(sizeof(*(resp->data))) );
  if( NULL == resp->data ) {
    syslog( LOG_DAEMON | LOG_WARNING, "Out of memory in add_line. "
	    "[%s:%u]\n", __FILE__, __LINE__ );
    exit(1);
  }
  if( *buffer == '+' ) { /* More lines follow */
    p = buffer+1;
    retval = 0;
  } else if( *buffer == '-' ) { /* no more lines follow */
    p = buffer+1;
    retval = 1;
  } else { /* Malformed Line. */
    syslog( LOG_DAEMON | LOG_NOTICE, "Malformed input %u. "
	    "[%s:%u]\n", resp->count, __FILE__, __LINE__ );
    p = buffer;
    retval = -1;
  }
  
  resp->data[resp->count-1] = strdup( p );
  if( NULL == resp->data[resp->count-1] ) {
    syslog( LOG_DAEMON | LOG_WARNING, "Out of memory in add_line. "
	    "[%s:%u]\n", __FILE__, __LINE__ );
    exit(1);
  }
  return retval;
}


/* (wuz)
 * Take a buffer which may end part way through a valid string
 * Output all the complete valid strings and return the number of characters
 * left (N) that are a partial string.
 * DOES alter buffer, ultimately should leave the first N characters
 * as being the partial string, this may result in buffer[0] == '\0'
 * (end wuz)
 */
int mangle_buffer( response_t *resp, char *buffer, int len )
{
  char *ptr = buffer;
  char *prev = buffer;
  int retval = 0;
  
  while( *ptr ) {
    if( *ptr == '\n' || *ptr == '\r' ) {
      *ptr = '\0';
      ptr++;
      while(*ptr && (*ptr == '\n' || *ptr == '\r' )) {
	ptr++;
      }
      /* so, now prev points to start of last line; and ptr points to
       * start of next line - parse and iterate
      */
      retval = add_line( resp, prev );
      if( retval > 0 ) { /* Message finished or error - stop iterating */
	return -1;
      } else if( retval < 0 ) { /* Malformed Line. Flag it */
	resp->status = MALFORMED;
      }
      prev = ptr;
    } else {
      ptr++;
    }
  }
  retval = 0;
  if( ptr != prev ) {
    memmove( buffer, prev, ptr - prev );
    buffer[ptr-prev] = '\0';
    retval = ptr - prev;
  } else {
    buffer[0] = '\0';
  }
  //  printf( "DBG: %p vs %p = %i; $$%s$$\n", prev, ptr, ptr - prev, buffer );
  return retval;
}

#define BUFSIZE 512
response_t *get_response( int fd, struct timeval *timeout )
{
  char buffer[BUFSIZE+1];
  char *p = buffer;
  response_t *resp = NULL;
  int size = 0;
  int last = 0;
  int retval = 0;
  int once = 0;
  
  if( NULL == ( resp = (response_t *)malloc(sizeof(*resp)) ) ) {
    syslog( LOG_DAEMON | LOG_ERR, "Out of memory in get_response(). "
	    "[%s:%u]\n", __FILE__, __LINE__ );
    exit(1);
  }
  resp->status = OKAY;
  resp->count = 0;
  resp->data = NULL;
  
  p = buffer;
  while( ( size = select_on( fd, p, BUFSIZE-last, timeout) ) > 0 ) {
    once = 1;
    retval = mangle_buffer( resp, buffer, last + size );
    if( retval < 0 ) { /* Input Finished Correctly */
      return resp;
    }

    /* Need more than 2 bytes to store next reply
     * BUFSIZE value = Design decision really.
     */
    if( BUFSIZE - retval < 2 ) {
      syslog( LOG_DAEMON | LOG_ERR, "Insufficient buffer space for reply. "
	      " send_request(). [%s:%u]. Buffer: %u\n", __FILE__, __LINE__,
	      BUFSIZE );
      exit( 1 );
    }
    last = retval; /* Typically 0 */
    p = &buffer[ last ];
  }
  
  if( last != 0 ) {
    if( -1 != (retval = mangle_buffer( resp, buffer, last )) )
      resp->status = ERROR;
    if( retval > 0 ) {
      retval = add_line( resp, buffer );
    }
    
  }
  
  if( once == 0 && size <= 0 ) { // Nothing Read and Timed Out
    resp->status = TIMEOUT_NOTHING;
  }
  if( once > 0 && size <= 0 ) { // Something Read, but Timed Out
    resp->status = TIMEOUT_DATA;
  }
  
  return resp;
}

void delete_response( response_t *response )
{
  int i = 0;
  if( response == NULL ) return;
  if( response->data == NULL ) return;
  for( i = 0; i < response->count; i++ ) {
    if( response->data[i] != NULL )
      free( response->data[i] );
    response->data[i] = NULL;
  }
  free( response->data );
  response->data = NULL;
}
