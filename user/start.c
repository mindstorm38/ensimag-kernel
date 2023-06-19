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
    while (1) {
        printf("Hello from user child process (%d)...\n", getpid());
        for (int i = 0; i < 500000000; i++);
    }
    return 42;
}

void user_start(void) {
    printf("Hello from user program...\n");
    start(user_child, 1024, 1, "user_child", NULL);
    start(user_child, 1024, 1, "user_child", NULL);
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
