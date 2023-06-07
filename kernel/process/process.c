#include "stdint.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"

#include "process.h"
#include "memory.h"
#include "cpu.h"


static struct process *running_process;
static pid_t pid_counter = 0;

// This context is never used at restoration.
static struct process_context dummy_context;


/// Function defined in assembly (process_context.S).
void process_context_switch(struct process_context *prev_ctx, struct process_context *next_ctx);

/// Internal function that actually exits.  We then schedule the next 
/// process.
static void process_internal_exit(void) {

    // Process is zombie until waited for by parent.
    running_process->state = PROCESS_ZOMBIE;

    // Remove the running process from its own linked list.
    struct process *prev_process = running_process->sched_prev;
    struct process *next_process = running_process->sched_next;

    // Special case, we can't do that.
    if (next_process == running_process || prev_process == running_process) {
        printf("weird case exiting last running process (TODO)\n");
        return;
    }

    prev_process->sched_next = next_process;
    next_process->sched_prev = prev_process;
    running_process = next_process;

    // TODO: Free stack/process? Maybe in waitpid because process 
    // still exists.

    process_context_switch(&dummy_context, &next_process->context);

}

/// Internal function that is implicitly called if the main function 
/// returns without calling exit. A process function returns an 
/// integer which will be placed in 'eax', we set this value to 
/// process' exit code before actually exiting.
static void process_implicit_exit(void) {
    // We need to ensure that eax is not overwritten by the prelude.
	__asm__ __volatile__("mov %%eax, %0" : "=r" (running_process->exit_code) : );
    process_internal_exit();
}

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
    stack_ptr[1] = (size_t) process_implicit_exit;
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
    process_context_switch(&dummy_context, &running_process->context);

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

void process_exit(int code) {
    running_process->exit_code = code;
    process_internal_exit();
    while(1);

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
