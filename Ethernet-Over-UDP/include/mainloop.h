#ifndef MAINLOOP_H
#define MAINLOOP_H

typedef void (*callback_t)(int fd);
void addRead(int fd,callback_t callback);
void remRead(int fd);
void mainloop(void);

#endif