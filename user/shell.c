#include "ensimag.h"
#include "shell.h"

#include "stdbool.h"
#include "string.h"
#include "stdlib.h"
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
        "Display system information, like processes and memory.",
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
        "[1-20]",
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
                    printf("\033eUsage:\033r %s \033b%s\033r\n", bt->name, bt->args);
                }
                builtin_found = true;
                break;
            }
        }

        if (!builtin_found) {
            printf("\033cCommand not found! Try 'help'...\033r\n");
        }

	}

	return 0;

}


static bool builtin_help(size_t argc, const char **args) {

    (void) args;
    if (argc != 1)
        return false;

    printf("\033eCommands:\033r\n");
    for (struct builtin *bt = builtins; bt->name != NULL; bt++) {
        if (bt->args[0] != 0) {
            printf("%8s \033b%s\033r\n", bt->name, bt->args);
            printf("         - %s\n", bt->description);
        } else {
            printf("%8s - %s\n", bt->name, bt->description);
        }
    }

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
    printf("%s (pid: %d, prio: %d)\n", name, pid, getprio(pid));

    int children[32];
    int children_count = getchildren(pid, children, 32);
    if (children_count <= 0)
        return;
    
    if (children_count > 32)
        children_count = 32;

    for (int i = 0; i < children_count; i++) {
        int child_pid = children[i];
        print_children_recursive(child_pid, indent + 2);
    }

}

static bool builtin_ps(size_t argc, const char **args) {

    (void) args;
    if (argc != 1)
        return false;

    printf("\033eProcesses:\033r\n");
    print_children_recursive(0, 2);

    printf("\033eMemory:\033r\n");
    unsigned int capacity;
    unsigned int used;
    system_memory_info(&capacity, &used);
    printf("  Usage: %d / %d Kio\n", used / 1024, capacity / 1024);

    return true;

}


static bool builtin_exit(size_t argc, const char **args) {
    
    (void) args;
    if (argc != 1)
        return false;

    system_power_off();
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

    int index = (int) arg;

    if (index == -1) {
        for (int i = 1; i <= 20; i++) {
            printf("\033e== RUNNING TEST %d ==\033r\n", i);
            int ret = test_run(i);
            if (ret != 0)
                return ret;
        }
        printf("\033e== TESTS COMPLETE ==\033r\n");
	    return 0;
    } else {
        return test_run(index);
    }

}

static bool builtin_test(size_t argc, const char **args) {

    int index = -1;
    if (argc == 2) {
        index = strtoul(args[1], NULL, 10);
        if (index < 1 || index > 20) {
            return false;
        }
    } else if (argc != 1) {
        return false;
    }

    int pid = start(test_wrapper, 4096, 128, "test_wrapper", (void *) index);
    waitpid(pid, NULL);

    return true;

}
