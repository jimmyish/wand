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
#include "protoverlay.h"

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


#define BUFSIZE 512
int send_request( char *one, char *two, char *three )
{
  char buffer[BUFSIZE+1];
  struct sockaddr_un sockname;
  struct timeval timeout;
  int fd = socket(PF_UNIX,SOCK_STREAM,0);
  response_t *resp = NULL;

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
  
  snprintf( buffer, BUFSIZE, "%s %s %s\n\r", one, (two)?two:"",
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

  if( NULL == (resp = get_response( fd, &timeout )) ) {
    close( fd );
    return 1;
  }

  print_response( resp, stdout );
  delete_response( resp );
  free( resp );

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
