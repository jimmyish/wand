#ifndef _WAND_H
#define _WAND_H

/* Function prototypes */ 
char *getword(char **buffer,int *len);
int load_module(char *filename);
void addEntry(char *mac,char *ip);
void send_update(int s, siginfo_t *t, void *m);
void clearOldEntries(void);
void doPacket(char *packet,int len);
void usage(const char *prog);

extern char *control_file_path;
extern int sock;
struct sockaddr_in serveraddr;
extern char macaddr[18];
extern struct sockaddr_in address;

struct node_t {
	        char *mac;
	        char *ip;
	        int flag; 	/* Flag
				 * 0 -- Seen in this packet, don't remove it
				 * 1 -- Not yet seen, remove it on the next pass
				 *                                       */
		struct node_t *next;
} *root;

#endif
