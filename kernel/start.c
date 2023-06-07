#include "debug/debugger.h"

#include "process.h"
#include "memory.h"
#include "clock.h"
#include "cga.h"
#include "cpu.h"

#include "stdio.h"


int idle(void *arg);
int proc1(void *arg);

void kernel_start(void) {

	cli();
	printf("\fKernel starting...\n");
	page_init();
	clock_init();
	sti();

	// process_idle(idle, 512, NULL);

	while (1)
	  hlt();

	return;
	
}


int idle(void *arg) {

	(void) arg;

	process_start(proc1, 512, "proc1", NULL);

	for (;;) {
		printf("[%s] pid = %i\n", process_name(), process_pid());
		for (int32_t i = 0; i < 100000000; i++);
		schedule();
	}

	return 0;

}

int proc1(void *arg) {
	(void) arg;
	printf("[%s] pid = %i\n", process_name(), process_pid());
	return 0;
}
