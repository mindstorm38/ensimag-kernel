#include "debug/debugger.h"

#include "segment.h"
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

void kernel_start(void) {

	printf("\fKernel starting...\n");
	page_init();
	pit_init();
	syscall_init();

	printf("Starting user program...\n");

	// syscall_jump_lower_pl(USER_CS, )
	// user_start(NULL);

	process_idle(idle, 512, NULL);

	while (1)
	  hlt();

	return;
	
}


/// Intended to run at user privilege (3).
int idle(void *arg) {

	(void) arg;

	while (1);

	// process_start(user_start, 1024, 127, "user", NULL);

	// for (;;) {

	// 	sti();
	// 	hlt();
	// 	cli();

	// }

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
