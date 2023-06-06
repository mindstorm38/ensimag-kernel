#include "debugger.h"
#include "stdio.h"
#include "time.h"
#include "cpu.h"
#include "cga.h"
#include "process.h"


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
