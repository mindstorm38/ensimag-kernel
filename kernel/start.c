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

void kernel_start(void) {

	printf("\fKernel starting...\n");
	page_init();
	rtc_init();
	pit_init();

	sti();
	// printf("eflags: %lu\n", save_flags());

	process_idle(idle, 512, NULL);

	while (1)
	  hlt();

	return;
	
}


int idle(void *arg) {

	(void) arg;

	process_start(proc1, 512, 0, "proc1", NULL);
	process_start(proc1, 512, 0, "proc2", NULL);

	for (;;) {
		printf("[%s]\n", process_name());
	}

	return 0;

}

int proc1(void *arg) {
	(void) arg;
	for (;;) {
		printf("[%s]\n", process_name());
	}
	return 0;
}
