#ifndef __ENSIMAG_H__
#define __ENSIMAG_H__

typedef int (*process_func_t)(void *);

int start(process_func_t pt_func, unsigned long ssize, int prio, const char *name, void *arg);
void exit(int retval) __attribute__((noreturn));
int getpid(void);
int getprio(int pid);
int chprio(int pid, int newprio);
int waitpid(int pid, int *retval);
int kill(int pid);
const char *getname(int pid);

void wait_clock(unsigned long clock);

int pcreate(int count);
int pdelete(int fid);
int psend(int fid, int message);
int preceive(int fid, int *message);
int pcount(int fid, int *count);
int preset(int fid);

void clock_settings(unsigned long *quartz, unsigned long *ticks);
unsigned long current_clock();

void cons_echo(int on);
int cons_read(char *string, unsigned long length);
void cons_write(const char *str, long size);

#endif
