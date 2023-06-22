#include "ensimag.h"
#include "shell.h"
#include "start.h"

#include "stddef.h"


// static int test_run_wrapper(void *arg);
// int test_run(int n);



// Idle process entry of the user space.
void user_start(void) {
	start(shell_start, 4096, 1, "shell", NULL);
	while (1);
}


// static int test_run_wrapper(void *arg) {
// 	(void) arg;
// 	for (int i = 20; i <= 20; i++) {
// 		printf("== RUNNING TEST %d ==\n", i);
// 		int ret = test_run(i);
// 		if (ret != 0)
// 			return ret;
// 	}
// 	printf("== TESTS COMPLETE ==\n");
// 	return 0;
// }
