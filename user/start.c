#include "start.h"
#include "stdio.h"
#include "ensimag.h"

// static int test_run_wrapper(void *arg);
// int test_run(int n);

void console_putbytes(const char *s, int len) {
    cons_write(s, len);
}

static int user_child(void *arg) {
    (void) arg;
    printf("Hello from user child process...\n");
    return 42;
}

void user_start(void) {
    printf("Hello from user program...\n");
    int pid = start(user_child, 1024, 2, "user_child", NULL);
    int retval;
    waitpid(pid, &retval);
    printf("Child pid: %d, exit: %d\n", pid, retval);
    while (1);
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
