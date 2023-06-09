#include "debug/debugger.h"

#include "process.h"
#include "memory.h"
#include "cga.h"
#include "cpu.h"
#include "rtc.h"
#include "pit.h"
#include "cmos.h"

#include "stdio.h"


static int idle(void *arg);
static int test_run_wrapper(void *arg);
int test_run(int n);


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

	process_start(test_run_wrapper, 512, 128, "test_run_wrapper", NULL);

	for (;;) {

		// Cleanup children.
		process_wait_pid(-1, NULL);

		sti();
		hlt();
		cli();

	}

	return 0;

}

static int test_run_wrapper(void *arg) {
	(void) arg;
	return test_run(2);
}
