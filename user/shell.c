#include "ensimag.h"
#include "shell.h"

#include "stdbool.h"
#include "string.h"
#include "stdio.h"


#define COMMAND_BUFFER_CAP 1024
#define ARGS_CAP 128


/// Builtin help command that displays all builtin commands.
static bool builtin_help(size_t argc, const char **args);
static bool builtin_ps(size_t argc, const char **args);
static bool builtin_exit(size_t argc, const char **args);
static bool builtin_echo(size_t argc, const char **args);
static bool builtin_test(size_t argc, const char **args);

struct builtin {
    const char *name;
    const char *args;
    const char *description;
    bool (*func)(size_t argc, const char **args);
};

struct builtin builtins[] = {
    {
        "help",
        "",
        "Displays all builtin commands.",
        builtin_help
    },
    {
        "ps",
        "",
        "Display hierarchy of active processes.",
        builtin_ps
    },
    {
        "exit",
        "",
        "Shutdown the kernel.",
        builtin_exit
    },
    {
        "echo",
        "<on|off>",
        "Enable echo (on) or not (off).",
        builtin_echo
    },
    {
        "test",
        "",
        "Run internal test suite.",
        builtin_test
    },
    { 0 }
};

static bool running = true;

int shell_start(void *arg) {

	(void) arg;

	char command_buffer[COMMAND_BUFFER_CAP];
    int read_len;

    const char *args[ARGS_CAP];

    // Enable echo by default.
	cons_echo(1);

	while (running) {

		printf("$ ");

		read_len = cons_read(command_buffer, COMMAND_BUFFER_CAP - 1);
        if (read_len == 0)
            continue;
        
        command_buffer[read_len] = 0; // Manually add EOL for cmp.
        
        const char **arg = args;
        size_t argc = 0;
        bool prev_space = true;

        for (char *c = command_buffer; *c != 0; c++) {
            if (*c == ' ') {
                prev_space = true;
                *c = 0;
            } else if (prev_space) {
                *arg = c;
                arg++;
                argc++;
                prev_space = false;
                // Max split reached.
                if (argc >= ARGS_CAP)
                    break;
            }
        }

        if (argc == 0)
            continue;

        bool builtin_found = false;
        const char *cmd = args[0];
        for (struct builtin *bt = builtins; bt->name != NULL; bt++) {
            if (strcmp(bt->name, cmd) == 0) {
                if (!bt->func(argc, args)) {
                    printf("Usage: %s %s\n", bt->name, bt->args);
                }
                builtin_found = true;
                break;
            }
        }

        if (!builtin_found) {
            printf("Command not found! Try 'help'...\n");
        }

	}

	return 0;

}


static bool builtin_help(size_t argc, const char **args) {

    (void) argc;
    (void) args;

    printf("\n");
    for (struct builtin *bt = builtins; bt->name != NULL; bt++) {
        if (bt->args[0] != 0) {
            printf("%8s %s\n", bt->name, bt->args);
            printf("         - %s\n", bt->description);
        } else {
            printf("%8s - %s\n", bt->name, bt->description);
        }
    }
    printf("\n");

    return true;

}


static void print_indent(int indent) {
    if (indent > 16)
        indent = 16;
    cons_write("                ", indent);
}

static void print_children_recursive(int pid, int indent) {

    char name[128];
    getname(pid, name, 128);
    print_indent(indent);
    printf("%s (%d)\n", name, pid);

    int children[32];
    int children_count = getchildren(pid, children, 32);
    if (children_count <= 0)
        return;
    
    if (children_count > 32)
        children_count = 32;

    for (int i = 0; i < children_count; i++) {
        int child_pid = children[i];
        print_children_recursive(child_pid, indent + 4);
    }

}

static bool builtin_ps(size_t argc, const char **args) {
    (void) argc;
    (void) args;
    print_children_recursive(0, 0);
    return true;
}


static bool builtin_exit(size_t argc, const char **args) {
    (void) argc;
    (void) args;
    cons_echo(0);
    running = false;
    return true;
}


static bool builtin_echo(size_t argc, const char **args) {
    
    if (argc != 2)
        return false;

    if (strcmp("on", args[1]) == 0) {
        cons_echo(1);
        printf("Read echo on...\n");
    } else if (strcmp("off", args[1]) == 0) {
        cons_echo(0);
        printf("Read echo off...\n");
    } else 
        return false;
    
    return true;

}


int test_run(int n);

static int test_wrapper(void *arg) {
	(void) arg;
	for (int i = 1; i <= 20; i++) {
		printf("== RUNNING TEST %d ==\n", i);
		int ret = test_run(i);
		if (ret != 0)
			return ret;
	}
	printf("== TESTS COMPLETE ==\n");
	return 0;
}

static bool builtin_test(size_t argc, const char **args) {

    (void) argc;
    (void) args;

    int pid = start(test_wrapper, 4096, 128, "test_wrapper", NULL);
    waitpid(pid, NULL);


    return true;

}
