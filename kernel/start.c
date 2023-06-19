#include "./debug/debugger.h"

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


void kernel_start(void) {

	printf("\fKernel starting...\n");
	page_init();
	pit_init();
	syscall_init();

	printf("Starting user program...\n");

	process_idle(user_start, 512, NULL);

	while (1)
	  hlt();

	return;
	
}
