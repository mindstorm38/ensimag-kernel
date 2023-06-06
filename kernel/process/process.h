#include "stdint.h"

#define PROCESS_NAME_MAX_SIZE 128
#define PROCESS_STACK_SIZE 512
#define PROCESS_MAX_COUNT 2


/// States that a process can take, used for scheduling.
enum process_state {
    PROCESS_ELECTED,
    PROCESS_AVAILABLE,
};

/// Registers context of a process.
struct process_context {
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
};

/// Stack structure used to aligned the stack to 16 bytes.
struct process_stack {
    char data[PROCESS_STACK_SIZE];
} __attribute__((aligned(16)));

/// The process structure.
struct process {
    int32_t pid;
    char name[PROCESS_NAME_MAX_SIZE];
    enum process_state state;
    struct process_context context;
    struct process_stack stack;
};

/// Get PID of the current process.
int32_t get_pid();
/// Get name of the current process.
char *get_name();
/// Schedule the next process to run.
void schedule();


void process_init();
