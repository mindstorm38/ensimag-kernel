#include "debug/debugger.h"

#include "process.h"
#include "memory.h"
#include "time.h"
#include "cga.h"
#include "cpu.h"

#include "stdio.h"


exit_code_t idle(void *arg);
exit_code_t proc1(void *arg);

void kernel_start(void) {

	// cli();
	// time_start();
	// sti();

	printf("\fKernel starting...\n");
	page_init();

	process_idle(idle, 512, NULL);

	while (1)
	  hlt();

	return;
	
}


exit_code_t idle(void *arg) {

	(void) arg;

	process_start(proc1, 512, "proc1", NULL);

	for (;;) {
		printf("[%s] pid = %i\n", process_name(), process_pid());
		for (int32_t i = 0; i < 100000000; i++);
		schedule();
	}

	return 0;

}

exit_code_t proc1(void *arg) {

	(void) arg;

	for (;;) {
		printf("[%s] pid = %i\n", process_name(), process_pid());
		for (int32_t i = 0; i < 100000000; i++);
		schedule();
	}

}
