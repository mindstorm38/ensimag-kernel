#include "debug/debugger.h"

#include "syscall.h"
#include "process.h"
#include "memory.h"
#include "start.h"
#include "cga.h"
#include "cpu.h"
#include "pit.h"
#include "cmos.h"

#include "stdio.h"


static int idle(void *arg);
// static int test_run_wrapper(void *arg);
// int test_run(int n);


void kernel_start(void) {

	printf("\fKernel starting...\n");
	page_init();
	pit_init();
	syscall_init();

	__asm__ __volatile__("int $49":::"memory");

	user_start(NULL);

	process_idle(idle, 512, NULL);

	while (1)
	  hlt();

	return;
	
}


int idle(void *arg) {

	(void) arg;

	// process_start(test_run_wrapper, 1024, 127, "test_run_wrapper", NULL);

	for (;;) {

		sti();
		hlt();
		cli();

	}

	return 0;

}

// static int test_run_wrapper(void *arg) {
// 	(void) arg;
// 	for (int i = 1; i <= 17; i++) {
// 		printf("== RUNNING TEST %d ==\n", i);
// 		int ret = test_run(i);
// 		if (ret != 0)
// 			return ret;
// 	}
// 	printf("== TESTS COMPLETE ==\n");
// 	return 0;
// }
