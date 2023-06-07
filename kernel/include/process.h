#include "stdint.h"
#include "stddef.h"

#define PROCESS_NAME_MAX_SIZE 128
#define PROCESS_STACK_SIZE 512
#define PROCESS_MAX_COUNT 2


/// States that a process can take, used for scheduling.
enum process_state {
    PROCESS_ACTIVE,
    PROCESS_AVAILABLE,
    PROCESS_WAIT_QUEUE,
    PROCESS_WAIT_SEMAPHORE,
    PROCESS_WAIT_IO,
    PROCESS_WAIT_CHILD,
    PROCESS_WAIT_TIME,
    PROCESS_ZOMBIE,
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
    /// Process id, its index in the internal process queue.
    int32_t pid;
    /// Name of the process.
    char name[PROCESS_NAME_MAX_SIZE];
    /// Next process to run when scheduling.
    struct process *next;
    /// State of the process.
    enum process_state state;
    /// Context saving registers that will be restored on context switch.
    struct process_context context;
    /// The process' stack.
    struct process_stack stack;
};

/// Get PID of the current process.
int32_t get_pid();
/// Get name of the current process.
char *get_name();

int32_t process_start(int32_t (*func)(void *), size_t stack_size, const char *name, void *arg);


/// Schedule the next process to run.
void schedule();


void process_init();
