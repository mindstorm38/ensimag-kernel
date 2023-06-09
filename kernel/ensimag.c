#include "stdint.h"

#include "ensimag.h"
#include "process.h"
#include "pit.h"
#include "cga.h"


void clock_settings(unsigned long *quartz, unsigned long *ticks) {
    pit_clock_settings((uint32_t *) quartz, (uint32_t *) ticks);
}

unsigned long current_clock() {
    return pit_clock();
}


void cons_write(const char *str, long size) {
    cga_write_bytes(str, size);
}


int start(int (*pt_func)(void *), unsigned long ssize, int prio, const char *name, void *arg) {
    return process_start(pt_func, ssize, prio, name, arg);
}

void exit(int retval) {
    process_exit(retval);
}

int getpid(void) {
    return process_pid();
}

int getprio(int pid) {
    return process_priority(pid);
}

int chprio(int pid, int newprio) {
    return process_set_priority(pid, newprio);
}
