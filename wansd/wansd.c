#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>

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

int addrcmp(struct sockaddr_in a,struct sockaddr_in b)
{
	if (a.sin_addr.s_addr != b.sin_addr.s_addr) {
		return a.sin_addr.s_addr-b.sin_addr.s_addr;
	}
	else {
		return a.sin_port-b.sin_port;
	}
}

int dopacket(char *buffer,int length,struct sockaddr_in address)
{
	struct tMapping *entry = findMapping(buffer);
	int update = 0;
	
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
	return update;
}

void sendupdate(int fd)
{
	struct tMapping *entry;
	char buffer[1024];
	char *b=buffer;

	/* Build the packet */
	for (entry = userList; entry; entry=entry->next) {
		/* Free expired entries after an hour */
		while (entry && time(NULL)-entry->lastseen>3600) {
			struct tMapping *entry2;
			entry2=entry->next;
			free(entry->mac);	
			free(entry);
			entry=entry2;
		}
		if (entry) {
			b+=sprintf(b,"%s",entry->mac)+1;
			b+=sprintf(b,"%s",inet_ntoa(entry->address.sin_addr))+1;
		}
	}
	for(entry = userList; entry; entry=entry->next) {
		sendto(fd,buffer,b-buffer,0,(struct sockaddr *)&entry->address,
		    	sizeof(entry->address));
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
	for (;;) {
		int addrlen=sizeof(address);
		char buffer[65536];
		int data=recvfrom(sock,(void *)buffer,sizeof(buffer),0,
		    	(struct sockaddr *)&address,&addrlen);
		if (data<0) {
			errors++;
			sleep(1);
			if (errors>10) {
				printf("recvfrom: %m\n");
				printf("Too many recvfrom errors, bailing.\n");
				return 1;
			}
		} else {
			errors=0;
		}
		dopacket(buffer,data,address);
		sendupdate(sock);
	};
	return 0;
}
