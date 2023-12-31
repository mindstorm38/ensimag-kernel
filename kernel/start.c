#include "./debug/debugger.h"

#include "keyboard.h"
#include "segment.h"
#include "syscall.h"
#include "process.h"
#include "memory.h"
#include "start.h"
#include "cons.h"
#include "cpu.h"
#include "pit.h"
#include "ps2.h"
#include "log.h"

#include "stdio.h"


void kernel_start(void) {

	printf("\f");
	page_init();
	pit_init();
	ps2_init();
	keyboard_init();
	syscall_init();
	printf(LOG_OK "Kernel ready\n\n");
	cons_start();

	process_idle(user_start, 512, NULL);

	while (1)
	  hlt();

	return;
	
}
