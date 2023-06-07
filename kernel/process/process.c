#include "stdint.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"

#include "process.h"
#include "memory.h"
#include "cpu.h"

// static struct process processes[PROCESS_MAX_COUNT];
// static size_t processes_count = 0;

static struct process *running_process;
static pid_t pid_counter = 0;

/// Function defined in assembly (process_context.S).
void process_context_switch(struct process_context *prev_ctx, struct process_context *next_ctx);

/// Internal function that allocate a process given
static struct process *process_alloc(process_entry_t entry, size_t stack_size, const char *name, void *arg) {

    // TODO: Check allocation errors.
    struct process *process = kalloc(sizeof(struct process));
    void *stack = kalloc(stack_size);

    // Initialize stack...
    process->stack = stack;

    void *stack_top = stack + stack_size;
    size_t *stack_ptr = stack_top - sizeof(size_t) * 3; // We add 3 words
    stack_ptr[0] = (size_t) entry;
    stack_ptr[1] = (size_t) process_exit;
    stack_ptr[2] = (size_t) arg;

    process->context.esp = (uint32_t) stack_ptr;

    // Initialize other fields
    strcpy(process->name, name);
    process->pid = pid_counter++;
    process->priority = 0;
    process->state = PROCESS_AVAILABLE;

    return process;

}

void process_idle(process_entry_t entry, size_t stack_size, void *arg) {
    
    running_process = process_alloc(entry, stack_size, "idle", arg);
    running_process->sched_next = running_process;
    running_process->sched_prev= running_process;

    // Switch to the next process while throwing the dummy context,
    // because we'll never return to the kernel's execution.
    struct process_context dummy_ctx;
    process_context_switch(&dummy_ctx, &running_process->context);

}

int32_t process_start(process_entry_t entry, size_t stack_size, const char *name, void *arg) {
    
    // assert(running_process)

    struct process *process = process_alloc(entry, stack_size, name, arg);

    // Insert the process in the schedule linked list, just after the
    // current process.
    process->sched_prev = running_process;
    process->sched_next = running_process->sched_next;
    running_process->sched_next = process;

    return process->pid;

}

void process_exit(void) {

}

int32_t process_pid(void) {
    return running_process->pid;
}

char *process_name(void) {
    return running_process->name;
}


void schedule() {

    struct process *prev_process = running_process;
    running_process = prev_process->sched_next;
    
    process_context_switch(&prev_process->context, &running_process->context);

}
