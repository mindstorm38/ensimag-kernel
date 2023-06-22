#include "ensimag.h"
#include "shell.h"

#include "stdbool.h"
#include "string.h"
#include "stdio.h"


#define COMMAND_BUFFER_CAP 1024
#define ARGS_CAP 128


/// Builtin help command that displays all builtin commands.
static void builtin_help(size_t argc, const char **args);
static void builtin_ps(size_t argc, const char **args);
static void builtin_exit(size_t argc, const char **args);

struct builtin {
    const char *name;
    const char *description;
    void (*func)(size_t argc, const char **args);
};

struct builtin builtins[] = {
    {
        "help",
        "Displays all builtin commands.",
        builtin_help
    },
    {
        "ps",
        "Display hierarchy of active processes.",
        builtin_ps
    },
    {
        "exit",
        "Shutdown the kernel.",
        builtin_exit,
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
                bt->func(argc, args);
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

static void builtin_help(size_t argc, const char **args) {

    (void) argc;
    (void) args;

    printf("\n");
    for (struct builtin *bt = builtins; bt->name != NULL; bt++) {
        printf("%6s - %s\n", bt->name, bt->description);
    }
    printf("\n");

}

static void builtin_ps(size_t argc, const char **args) {

    (void) argc;
    (void) args;

}

static void builtin_exit(size_t argc, const char **args) {
    (void) argc;
    (void) args;
    cons_echo(0);
    running = false;
}
