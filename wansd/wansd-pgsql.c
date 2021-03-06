#ifndef LINUX
#include <sys/types.h>
#else
#include <malloc.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <stdlib.h>
#include <postgresql/libpq-fe.h>
#include "daemons.h"


PGconn *conn = 0;

struct tMapping {
  struct tMapping *next;
  char *mac;
  struct sockaddr_in address;
  int version;
  time_t lastseen;
} *userList = NULL;

struct tMapping *findMapping(char *mac) {
  struct tMapping *tmp = userList;
  while (tmp && strcmp(tmp->mac,mac)!=0)
    tmp=tmp->next;
  return tmp;
}

void printlist()
{
  struct tMapping *entry = userList;
  FILE *logfile = fopen("/var/tmp/wand_table","w");  
  
  syslog(LOG_DEBUG, "Current Time: %d\n", (int) time(NULL));
  fprintf(logfile, "Current Time: %d\n", (int) time(NULL));
  
  syslog(LOG_DEBUG, "MAC ADDR Last Seen\n");
  fprintf(logfile, "MAC ADDR Last Seen\n");
  
  
  for(entry = userList; entry; entry=entry->next) {
    syslog(LOG_DEBUG, "%s %s %d\n", entry->mac, 
	   inet_ntoa(entry->address.sin_addr), (int) entry->lastseen);
    fprintf(logfile, "%s %s %d\n", entry->mac,
  	   inet_ntoa(entry->address.sin_addr), (int) entry->lastseen);
  }

  fclose(logfile);
  return;
}

int addrcmp(struct sockaddr_in a,struct sockaddr_in b)
{
  if (a.sin_addr.s_addr < b.sin_addr.s_addr) {
    return -1;
  }
  else if (a.sin_addr.s_addr == b.sin_addr.s_addr) {
    if (a.sin_port < b.sin_port) {
      return -1;
    }
    else if (a.sin_port == b.sin_port) {
      return 0;
    }
    else {
      return +1;
    }
  }
  else {
    return +1;
  }
}

/* Returns NULL if you have to send an update to everyone,
 * or the mapping to send it to. :)
 */
struct tMapping *dopacket(char *buffer,int length,struct sockaddr_in address)
{
  struct tMapping *entry;
  char sqlstring[128];
  int update = 0;
  PGresult *res;
  char *cp;
  for (cp=buffer;*cp;cp++)
	  *cp=tolower(*cp);
  entry = findMapping(buffer);
  bzero(sqlstring,128);
  if (!entry) {
    update = 1;
    entry = malloc(sizeof(struct tMapping));
    entry->next = userList;
    userList=entry;
    entry->mac=strdup(buffer);
  }
  if (addrcmp(entry->address,address)!=0) {
    entry->address = address;
    update = 1;
  }
  entry->lastseen = time(NULL);
  entry->version = 1;
  syslog(LOG_DEBUG,"Recieved update from '%s' (%s)",
	 entry->mac,
	 inet_ntoa(entry->address.sin_addr)
	 );
  
  if (conn != 0) {
    snprintf(sqlstring, 128, "DELETE FROM wand WHERE mac = '%s'",entry->mac);
    res = PQexec(conn, sqlstring);
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
      syslog(LOG_DEBUG, "ERROR inserting into wand database");
      PQclear(res);
      PQfinish(conn);
      conn = 0;
    } else {
    
    snprintf(sqlstring, 128, "INSERT INTO wand  (mac,ip) VALUES ('%s','%s')",entry->mac,inet_ntoa(entry->address.sin_addr));
    res = PQexec(conn,sqlstring);
    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
       syslog(LOG_DEBUG, "ERROR inserting into wand database");
       PQclear(res);
       PQfinish(conn);
       conn = 0;
    }
    }

     
  }
  
  return (!update) ? entry : NULL;
}

/* This should send the updates to the target (if the target is NULL
 * then send the update to everyone.
 * The update can fill the output buffer, in this case the program
 * dies. THIS IS A BUG ! There should be multiple update packets sent
 * in this case.
 */

void sendupdate(int fd,struct tMapping *target)
{
  char buffer[1024];
  char *b=buffer;
  char sqlstring[128];
  struct tMapping *entry=NULL;
  struct tMapping *last=NULL;
  PGresult   *res;

  /* Build the packet */
  for (entry = userList; entry; entry=entry->next) {
    /* Free expired entries after an hour */
    while (entry && time(NULL)-entry->lastseen>3600) {
      struct tMapping *entry2;
      
      syslog(LOG_DEBUG,"Expired: '%s' (%s)", entry->mac,
	     inet_ntoa(entry->address.sin_addr));

      /* expire from SQL db as well */
      if (!conn) {
	snprintf(sqlstring, 128, "DELETE FROM wand WHERE mac = '%s'",entry->mac);
	res = PQexec(conn, sqlstring);
        if (!res || PQresultStatus(res) != PGRES_COMMAND_OK) {
	  syslog(LOG_DEBUG, "ERROR deleting from wand database");
	  PQclear(res);
	 // PQfinish(conn);
	 // conn = 0;
	}
      
      }
      
      entry2=entry->next;
      free(entry->mac);	
      free(entry);
      if (last) {
	last->next = entry2;
      }
      else {
	userList = entry2;
      } 	
      entry=entry2;
    }
    if (entry) {
      b+=snprintf(b, (int) (buffer+1022-b), "%s", entry->mac)+1;
      b+=snprintf(b, (int) (buffer+1022-b), "%s", 
		  inet_ntoa(entry->address.sin_addr))+1;
    }
    
    last = entry;
  }

  if ((b - buffer + 1023) <= 0) {
    syslog(LOG_DEBUG,"Buffer overflow for update packet. Dying !\n");
    exit(0);
  }
  
  if (!target) {
    for(entry = userList; entry; entry=entry->next) {
      sendto(fd,buffer,b-buffer,0,
	     (struct sockaddr *)&entry->address,sizeof(entry->address));
    }
  }
  else {
    sendto(fd,buffer,b-buffer,0,(struct sockaddr *)&target->address,
	   sizeof(target->address));
  }
}

int main(int argc,char **argv)
{
  int sock = socket(PF_INET, SOCK_DGRAM, 0);
  struct sockaddr_in address;
  int errors=0;
  
  if (sock<0) {
    perror("socket");
    return 1;
  };
  
  address.sin_family = AF_INET;
  address.sin_port = htons(44444);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(sock,(struct sockaddr *)&address,sizeof(address))<0) {
    perror("bind");
    return 1;
  }
  
  signal(SIGUSR1, printlist);
  
  daemonise(argv[0]);
  put_pid("wansd");
  openlog(argv[0],LOG_PID,LOG_DAEMON);
  syslog(LOG_NOTICE,"%s started.",argv[0]);
 
/* open database connetion */
  conn = PQconnectdb("hostaddr=127.0.0.1 dbname=metanet user=www-data password=www-data");
  if (PQstatus(conn) == CONNECTION_BAD) {
    syslog(LOG_DEBUG, "Connection to database '%s' failed.\n", "metanet");
    syslog(LOG_DEBUG, "%s", PQerrorMessage(conn));
    PQfinish(conn);
    conn = 0;
  }

 
  
  for (;;) {
    int addrlen=sizeof(address);
    char buffer[65536];
    int data=recvfrom(sock,(void *)buffer,sizeof(buffer),0,
		      (struct sockaddr *)&address,&addrlen);
    if (data<0) {
      errors++;
      sleep(1);
      if (errors>10) {
	syslog(LOG_ERR,"recvfrom: %m\n");
	syslog(LOG_ERR,"Too many errors, bailing.\n");
	return 1;
      }
    } else {
      errors=0;
    }
    sendupdate(sock,dopacket(buffer,data,address));
  };
  return 0;
}




