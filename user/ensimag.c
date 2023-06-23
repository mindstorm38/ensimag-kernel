// Kernel API (syscall).
//
// Note that we adjust process priority in this wrappers, because the
// kernel's priorities are in range 0 to 255 included, but the ensimag
// API uses 1 to 256 included.

#include "syscall.h"
#include "stdint.h"

#include "syscall_shared.h"
#include "ensimag.h"


int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg) {
    return syscall5(
        SC_PROCESS_START,
        (size_t) pt_func, 
        (size_t) ssize, 
        (size_t) prio - 1, 
        (size_t) name, 
        (size_t) arg);
}

void exit(int retval) {
    syscall1(SC_PROCESS_EXIT, retval);
    while(1);
}

int getpid(void) {
    return syscall0(SC_PROCESS_PID);
}

int getprio(int pid) {
    int prio = syscall1(SC_PROCESS_PRIORITY, pid);
    return prio < 0 ? prio : (prio + 1);
}

int chprio(int pid, int newprio) {
    int prev_prio = syscall2(SC_PROCESS_SET_PRIORITY, pid, newprio - 1);
    return prev_prio < 0 ? prev_prio : (prev_prio + 1);
}

int waitpid(int pid, int *retval) {
    return syscall2(SC_PROCESS_WAIT, pid, (size_t) retval);
}

int kill(int pid) {
    return syscall1(SC_PROCESS_KILL, pid);
}

int getname(int pid, char *dst, int count) {
    return syscall3(SC_PROCESS_NAME, pid, (size_t) dst, count);
}

int getchildren(int pid, int *children_pids, int count) {
    return syscall3(SC_PROCESS_CHILDREN, pid, (size_t) children_pids, count);
}

int getstate(int pid) {
    return syscall1(SC_PROCESS_STATE, pid);
}

void wait_clock(unsigned long clock) {
    syscall1(SC_PROCESS_WAIT_CLOCK, clock);
}

int pcreate(int count) {
    return syscall1(SC_PROCESS_QUEUE_CREATE, count);
}

int pdelete(int fid) {
    return syscall1(SC_PROCESS_QUEUE_DELETE, fid);
}

int psend(int fid, int message) {
    return syscall2(SC_PROCESS_QUEUE_SEND, fid, message);
}

int preceive(int fid, int *message) {
    return syscall2(SC_PROCESS_QUEUE_RECEIVE, fid, (size_t) message);
}

int pcount(int fid, int *count) {
    return syscall2(SC_PROCESS_QUEUE_COUNT, fid, (size_t) count);
}

int preset(int fid) {
    return syscall1(SC_PROCESS_QUEUE_RESET, fid);
}

void clock_settings(unsigned long *quartz, unsigned long *ticks) {
    syscall2(SC_CLOCK_SETTINGS, (size_t) quartz, (size_t) ticks);
}

unsigned long current_clock() {
    return syscall0(SC_CLOCK_GET);
}


void cons_write(const char *str, long size) {
    syscall2(SC_CONSOLE_WRITE, (size_t) str, size);
}

int cons_read(char *string, unsigned long length) {
    return syscall2(SC_CONSOLE_READ, (size_t) string, (size_t) length);
}

void cons_echo(int on) {
    syscall1(SC_CONSOLE_ECHO, on);
}

void console_putbytes(const char *s, int len) {
    cons_write(s, len);
}


int system_memory_info(unsigned int *capacity, unsigned int *used) {
    return syscall2(SC_SYSTEM_MEMORY_INFO, (size_t) capacity, (size_t) used);
}

void system_power_off(void) {
    syscall0(SC_SYSTEM_POWER_OFF);
}
