/* Wand Project - client
 * $Id$
 * Licensed under the GPL, see file COPYING in the top level for more
 * details.
 */

#include <sys/socket.h>
#include <sys/types.h> /* for select */
#include <sys/time.h> /* for select */
#include <sys/un.h>
#include <stdio.h>
#include <ctype.h> /* for isspace */
#include <stdlib.h> /* for malloc */
#include <unistd.h> /* for getopt, select */
#include <getopt.h> /* for getopt */

static int rude = 0;

void usage( char *pname ) 
{
	printf( "\
Usage:\n\
%s [-h|-v|-c|-l|-a mac ip|-d mac]\n\
Where:\n\
	-h	This Help Screen\n\
	-v	Query for the version of Etud\n\
	-R	Be RUDE - for testing ONLY\n\
	-c	Conversation mode, enter Queries interactively<tm>\n\
	-l	Query for a list of MAC addresses and IP addresses\n\
	-a	Query to add the given MAC address and IP address\n\
		Mutually exclusive with -d\n\
	-d	Query to delete the given MAC address\n\
		Mutually exclusive with -a\n\
MAC address must be in the format 00:aa:bb:cc:dd:ee, using either : or -\n\
as the delimiter.\n\
IP address must be in the format 1.127.255.0, using only . as the delimiter\n\
",pname);
	exit(0);
}              

int original_main( int argc, char **argv )
{
	char buf[1024];
	int size;
	struct sockaddr_un sockname;
	int fd=socket(PF_UNIX,SOCK_STREAM,0);
	if (fd<0) {
		perror("control socket");
		return 1;
	}
	sockname.sun_family = AF_UNIX;
	strcpy(sockname.sun_path,"/var/run/Etud.ctrl");
	if (connect(fd,(const sockaddr *)&sockname,sizeof(sockname))<0) {
		perror("control connect");
		close(fd);
		return -1;
	}
	if (write(fd,argv[1],strlen(argv[1]))!=(signed)strlen(argv[1]) || write(fd,"\r\n",2)!=2)
		return 2;
	alarm(2);
	while ((size=read(fd,buf,1024))>1)
		write(1,buf,size);
	return 0;
}

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
		if( NULL == ( to = (struct timeval *)malloc( sizeof( struct
		    timeval ) ) ) ) {
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

/* displays a reply to the screen. Pretty simple/crude 
 * return 0 to continue iterating, or negative to terminate the while.
 */
int display_reply( char *buffer ) 
{
	int size = strlen( buffer );
	/* Inelegant Hack - remove trailing ctype-ish space */
	while( size > 1 && 
	       isspace( buffer[ size-1 ] ) ) {
		buffer[ size-1 ] = '\0';
		size--;
	}
	
	if( strlen( buffer ) <= 1 ) {
		printf("Boring / Blank line from server. no fun.\n");
		printf( "please strace this program and if you get this"
		        "message again, send me a copy of the output!\n"
			" -- Gerard\n" );
		return 0;
	}

	if( buffer[0] == '+' ) { /* Continuation */
		printf( "\"%s\"\n", buffer+1 );
		return 0;
	} else if( buffer[0] == '-' ) { /* Last */
		printf( "'%s'\n", buffer+1 );
		return -1;
	} else { /* malformed */
		printf( "Unexpected reply from server, \"%s\"\n",
			buffer );
		return 0;
	}
	printf( "Shouldn't get here." );
	return 0;
}

/* (wuz)
 * Take a buffer which may end part way through a valid string
 * Output all the complete valid strings and return the number of characters
 * left (N) that are a partial string.
 * DOES alter buffer, ultimately should leave the first N characters
 * as being the partial string, this may result in buffer[0] == '\0'
 * (end wuz)
 */
int mangle_buffer( char *buffer, int len )
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
      retval = display_reply( prev );
      if( retval < 0 ) { /* Message finished or error - stop iterating */
	return retval;
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
int send_request( char *one, char *two, char *three )
{
  char buffer[BUFSIZE+1];
  char *p = buffer;
  int size = 0;
  int last = 0;
  int retval = 0;
  struct sockaddr_un sockname;
  struct timeval timeout;
  int fd = socket(PF_UNIX,SOCK_STREAM,0);
  
  if( fd < 0 ) {
    perror("control socket");
    return 1;
  }
  
  sockname.sun_family = AF_UNIX;
  strcpy(sockname.sun_path,"/var/run/Etud.ctrl");
  
  if (connect(fd,(const sockaddr *)&sockname,sizeof(sockname))<0) {
    perror("control connect");
    close(fd);
    return -1;
  }
  
  snprintf( buffer, 512, "%s %s %s\n\r", one, (two)?two:"",
	    (three)?three:"" );
  if( write(fd,buffer,strlen(buffer)) != (signed)strlen(buffer) )
    return 2;
  
  if( rude > 0 ) {
    printf( "Wrote \"%s\".\n Rudely closing fd and leaving.\n",
	    buffer );
    close( fd );
    return 0;
  }
  
  /* two second timeout between packets today */
  timeout.tv_usec = 0;
  timeout.tv_sec = 2;
  
  p = buffer;
  while( ( size = select_on( fd, p, BUFSIZE-last, &timeout) ) > 0 ) {
    
    retval = mangle_buffer( buffer, last + size );
    if( retval < 0 ) {
      close( fd );
      return 0;
    }
    /* Need more than 2 bytes to store next reply
     * BUFSIZE value = Design decision really.
     */
    if( BUFSIZE - retval < 2 ) {
      fprintf( stderr, "Insufficient buffer space / reply from server too long"
	       " in send_request(). Buffer: %u\n", BUFSIZE );
      exit( 1 );
    }
    last = retval; /* Typically 0 */
    p = &buffer[ last ];
  }
  if( last != 0 )
    mangle_buffer( buffer, last );
  close( fd );
  return 0;
}


int main( int argc, char *argv[] )
{
	int o;
	int any_arg = 0;
	int any_add = 0;
	int any_del = 0;

	static struct option long_options[] =
	{
	/* {name,argument,flag,value} */
	{"help",0,0,'h'},
	{"version",0,0,'v'},
	{"Rude",0,0,'R'},
	{"list",0,0,'l'},
	{"converse",0,0,'c'},
	{"add",0,0,'a'},
	{"delete",0,0,'d'},
	{0,0,0,0}
	};

	while( 1 ) {
	  //	  printf( "\n!!! argv=\"%p\" \n", argv );
		o = getopt_long( argc, argv, "hvRlcad", long_options, NULL );
		if( o == -1 )
			break;
		switch( o ) {
		case 'h':
			usage(argv[0]);
			return 0;
		case 'v':
			any_arg++;
			send_request( "VERSION", NULL, NULL );
			break;
		case 'R':
			rude++;
			printf( "How Rude!\n" );
			break;
		case 'l':
			any_arg++;
			send_request( "LIST", NULL, NULL );
			break;
		case 'c':
			any_arg++;
			break;
		case 'a':
			any_arg++;
			if( any_del > 0 ) {
				fprintf( stderr, "I'm sorry, mixing -a and "
					"-d is illegal.\n" );
				return 1;
			}
			if( any_add > 0 ) {
				fprintf( stderr, "I'm sorry, multiple -a "
					"options are illegal.\n" );
				return 1;
			}
			any_add++;
			break;
		case 'd':
			any_arg++;
			if( any_add > 0 ) {
				fprintf( stderr, "I'm sorry, mixing -d and "
					"-a is illegal.\n" );
				return 1;
			}
			if( any_del > 0 ) {
				fprintf( stderr, "I'm sorry, multiple -d "
					"options are illegal.\n" );
				return 1;
			}
			any_del++;
			break;
		default:
			fprintf( stderr, "Unknown or unrecognised "
				"option.\n" );
			usage( argv[0] );
			return 0;
		}
	}
	if( any_arg == 0 ) {
		fprintf( stderr, "No command line options offered.\n");
		usage( argv[0] );
		return 0;
	}

	if( any_add > 0 || any_del > 0 ) { /* add or delete */
		if( (any_del>0 && argc - optind == 1 ) ||
		    (any_add>0 && argc - optind == 2 ) ) {
			if( any_add > 0 )
				send_request( "ADD", argv[optind], 
					     argv[optind+1] );
			else
				send_request( "DEL", argv[optind], NULL );
		} else {
			fprintf(stderr, "Option -%c requires %u "
				"argument%s!\n", (any_add>0)?'a':'d',
				(any_add>0)?2:1, (any_add>0)?"s":"");
			return 1;
		}
	}
	return 0;
}
