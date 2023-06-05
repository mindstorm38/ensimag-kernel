#include "debugger.h"
#include "stdio.h"
#include "cpu.h"
#include "cga.h"


static int fact(int n) {
	if (n < 2)
		return 1;

	return n * fact(n-1);
}

void kernel_start(void) {

	printf("\fKernel starting...\n");

	int i;
	// call_debugger(); useless with qemu -s -S

	i = 10;

	i = fact(i);

	printf("fact(10) = %d\n", i);

	while(1)
	  hlt();

	return;
	
}
