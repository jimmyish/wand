#ifndef DRIVER_H
#define DRIVER_H

#ifdef __cplusplus
extern "C" {
#endif

struct interface_t {
	char *name;
	char *version;
	int (*setup)(unsigned long myid);
	int (*down)(void);
	int (*read)(char *frame,int length);
	int (*write)(char *frame,int length);
};

void register_device(struct interface_t *interface_description);
struct interface_t *find_interface(char *s);
int init_interface(struct interface_t *interface,int id);
void send_interface(char *buffer,int size);

#ifdef __cplusplus
}
#endif

#endif