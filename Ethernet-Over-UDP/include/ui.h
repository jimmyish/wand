#ifndef UI_H
#define UI_H

int ui_setup(char *s="/var/run/Etud.ctrl");
int ui_process(int fd);
int ui_process_request(int fd);
int ui_send(int sock,char *msg);

#endif