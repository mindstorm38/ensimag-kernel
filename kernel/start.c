#include "debug/debugger.h"

#include "process.h"
#include "memory.h"
#include "time.h"
#include "cga.h"
#include "cpu.h"

#include "stdio.h"
#include <stdio.h>


void kernel_start(void) {

	// cli();
	// time_start();
	// sti();

	printf("\fKernel starting...\n");
	page_init();

	// process_init();

	while (1)
	  hlt();

	return;
	
}
