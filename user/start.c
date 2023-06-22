#include "ensimag.h"
#include "shell.h"
#include "start.h"

#include "stddef.h"


// Idle process entry of the user space.
void user_start(void) {
	
	start(shell_start, 8192, 1, "shell", NULL);
	while (1);
	
}