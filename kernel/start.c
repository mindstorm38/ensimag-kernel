#include "stdio.h"
#include "debug/debugger.h"
#include "peripheral/time.h"
#include "peripheral/cga.h"
#include "process/process.h"
#include "cpu.h"


void kernel_start(void) {

	cli();
	time_start();
	sti();

	printf("\fKernel starting...\n");

	process_init();

	while (1)
	  hlt();

	return;
	
}
