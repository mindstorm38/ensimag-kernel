#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "stdint.h"
#include "stddef.h"

#define PROCESS_NAME_MAX_SIZE 128
#define PROCESS_STACK_SIZE 512
#define PROCESS_MAX_COUNT 2
#define PROCESS_MAX_PRIORITY 256



typedef int32_t pid_t;

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
    /// Name of the process.
    char name[PROCESS_NAME_MAX_SIZE];
    /// Previous process in the schedule ring for the current priority.
    struct process *sched_prev;
    /// Next process in the schedule ring for the current priority.
    struct process *sched_next;
    /// Pointer to the parent process.
    struct process *parent;
    /// Head of the child linked list.
    struct process *child;
    /// Next child of the parent process's child linked list.
    struct process *sibling;
    /// State of the process.
    enum process_state state;
    /// Context saving registers that will be restored on context switch.
    struct process_context context;
    /// The process' stack.
    char *stack;
    /// Process id, its index in the internal process queue.
    pid_t pid;
    /// Scheduling priority, from 0 to PROCESS_MAX_PRIORITY excluded.
    int priority;
    /// Valid when the process is a zombie, it's used for returning
    /// the exit code to the parent that waits for it.
    int exit_code;
};

/// Type alias for process entry point.
typedef int (*process_entry_t)(void *);

/// Startup function that starts the first process. It should be 
/// called only once at kernel startup, this process can then starts
/// other threads.
void process_idle(process_entry_t entry, size_t stack_size, void *arg);
/// Start a process.
pid_t process_start(process_entry_t entry, size_t stack_size, int priority, const char *name, void *arg);
/// Exit from the current process.
void process_exit(int code) __attribute__((noreturn));
/// Get PID of the current process.
pid_t process_pid(void);
/// Get name of the current process.
char *process_name(void);

/// Schedule the next process to run.
void schedule(void);

#endif
