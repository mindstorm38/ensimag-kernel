#include "debug/debugger.h"
#include "stdio.h"
#include "process.h"
#include "time.h"
#include "cga.h"
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
