#include "debug/debugger.h"

#include "process.h"
#include "memory.h"
#include "cga.h"
#include "cpu.h"
#include "rtc.h"
#include "pit.h"
#include "cmos.h"

#include "stdio.h"


int idle(void *arg);
int proc1(void *arg);
int proc2(void *arg);


void kernel_start(void) {

	printf("\fKernel starting...\n");
	page_init();
	rtc_init();
	pit_init();

	process_idle(idle, 512, NULL);

	while (1)
	  hlt();

	return;
	
}


int idle(void *arg) {

	(void) arg;

	process_start(proc1, 512, 0, "proc1(1)", NULL);
	process_start(proc1, 512, 0, "proc1(2)", NULL);

	process_start(proc2, 512, 0, "proc2", NULL);

	pid_t wait_pid = process_wait_pid(-1, NULL);

	printf("[%s] waited pid: %d\n", process_name(), wait_pid);

	for (;;) {
		printf("[%s] pid: %d\n", process_name(), process_pid());
		// process_debug();
		for (int i = 0; i < 100000000; ++i);
		sti();
		hlt();
		cli();
	}

	return 0;

}

int proc1(void *arg) {

	(void) arg;

	for (int i = 0;; i++) {
		printf("[%s] pid: %d\n", process_name(), process_pid());
		// process_debug();
		for (int i = 0; i < 100000000; ++i);
		sti();
		hlt();
		cli();
	}

	return 0;

}

int proc2(void *arg) {
	(void) arg;
	for (int i = 0; i < 5; ++i) {
		sti();
		hlt();
		cli();
	}
	printf("[END OF %s] pid: %d, prio: %d\n", process_name(), process_pid(), process_priority(process_pid()));
	return 0;
}
