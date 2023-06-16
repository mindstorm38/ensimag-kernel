#include "start.h"
#include "stdio.h"
#include "ensimag.h"

// static int test_run_wrapper(void *arg);
// int test_run(int n);

void console_putbytes(const char *s, int len) {
    cons_write(s, len);
}

void user_start(void) {
    printf("Hello from user program...\n");
}


// static int test_run_wrapper(void *arg) {
// 	(void) arg;
// 	for (int i = 1; i <= 17; i++) {
// 		printf("== RUNNING TEST %d ==\n", i);
// 		int ret = test_run(i);
// 		if (ret != 0)
// 			return ret;
// 	}
// 	printf("== TESTS COMPLETE ==\n");
// 	return 0;
// }