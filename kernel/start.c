#include "./debug/debugger.h"

#include "keyboard.h"
#include "segment.h"
#include "syscall.h"
#include "process.h"
#include "memory.h"
#include "start.h"
#include "cga.h"
#include "cpu.h"
#include "pit.h"
#include "ps2.h"

#include "stdio.h"


void kernel_start(void) {

	printf("\f[ .. ] Kernel starting...\n");
	page_init();
	pit_init();
	ps2_init();
	keyboard_init();
	syscall_init();
	printf("[ \aaOK\ar ] Kernel ready\n");

	printf("\n\n\n");

	process_idle(user_start, 512, NULL);

	while (1)
	  hlt();

	return;
	
}
