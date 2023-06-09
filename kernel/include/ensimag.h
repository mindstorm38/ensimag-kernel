#ifndef __ENSIMAG_H__
#define __ENSIMAG_H__

void clock_settings(unsigned long *quartz, unsigned long *ticks);
unsigned long current_clock();

void cons_echo(int on);
int cons_read(char *string, unsigned long length);
void cons_write(const char *str, long size);

int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg);
void exit(int retval);
int getpid(void);
int getprio(int pid);
int kill(int pid);
int chprio(int pid, int newprio);

void wait_clock(unsigned long clock);
int waitpid(int pid, int *retval);

int pcount(int fid, int *count);
int pcreate(int count);
int pdelete(int fid);
int preceive(int fid, int *message);
int preset(int fid);
int psend(int fid, int message);

#endif
