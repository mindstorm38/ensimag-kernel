#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"

#include "process.h"
#include "memory.h"
#include "cpu.h"
#include <stdio.h>


static struct process *process_sched_rings[PROCESS_MAX_PRIORITY];
static struct process *running_process;
static pid_t pid_counter = 0;

// This context is never used at restoration.
static struct process_context dummy_context;


/// Function defined in assembly (process_context.S).
void process_context_switch(struct process_context *prev_ctx, struct process_context *next_ctx);

/// Internal function used to insert a process in its scheduler ring.
static void process_sched_ring_insert(struct process *process) {

    // Get the first process in this ring.
    struct process *first_process = process_sched_rings[process->priority];
    
    if (first_process) {
        process->sched_next = first_process;
        process->sched_prev = first_process->sched_prev;
        first_process->sched_prev = process;
    } else {
        // It's the first process, it has its own ring.
        process->sched_next = process;
        process->sched_prev = process;
    }

    process_sched_rings[process->priority] = process;

}

/// Internal function used to remove a process from its scheduler ring.
/// Returns the next process after the removed one, might be null if
/// the ring is now empty.
static struct process *process_sched_ring_remove(struct process *process) {

    // Remove the running process from its own ring.
    struct process *prev_process = process->sched_prev;
    struct process *next_process = process->sched_next;

    // If the process is it's own next process, it means that it was
    // alone in its ring. So we set all to NULL.
    if (next_process == process) {
        next_process = NULL;
        prev_process = NULL;
    } else {
        prev_process->sched_next = next_process;
        next_process->sched_prev = prev_process;
    }

    // First process of the ring.
    struct process **first_process_ptr = &process_sched_rings[process->priority];
    if (*first_process_ptr == process) {
        *first_process_ptr = next_process;
    }

    return next_process;

}

/// Internal function used to find a non-null schedule ring up to
/// and excluding the given priority. This should not return null
/// since at least ring 0 should contain idle process.
static struct process *process_sched_ring_find(int max_priority) {

    for (int priority = max_priority - 1; priority >= 0; priority--) {
        if (process_sched_rings[priority]) {
            return process_sched_rings[priority];
        }
    }

    // Should not happen, TODO: panic?
    return NULL;

}

/// Internal function called from timer interrupt routine.
/// It's responsible for scheduling the next process to run.
/// 
/// The caller must set 'running_process' state manually before 
/// calling this function, because the scheduling can vary depending
/// on its source (preemptive timer, I/O, etc.).
/// 
/// Next process can optionally be specified, this is compatible with
/// ring remove. If both are true/non-null then the function don't try 
/// to find a lower-priority ring.
static void process_schedule(bool ring_remove, struct process *init_next_process) {

    struct process *prev_process = running_process;
    struct process *next_process = init_next_process;

    // Start by removed the process from the ring.
    if (ring_remove) {

        struct process *new_next_process = process_sched_ring_remove(prev_process);
        
        // Next process is not forced.
        if (next_process == NULL) {
            next_process = new_next_process;
            // The current ring is empty, find a lower-priority ring.
            if (next_process == NULL) {
                next_process = process_sched_ring_find(prev_process->priority);
            }
        }

    } else if (next_process == NULL) {
        next_process = prev_process->sched_next;
    }

    running_process = next_process;
    running_process->state = PROCESS_ACTIVE;

    process_context_switch(&prev_process->context, &next_process->context);

}

/// Internal function that actually exits.  We then schedule the next 
/// process.
static void process_internal_exit(void) {

    // TODO: Free stack/process? Maybe in waitpid...

    // Process is zombie until waited for by parent.
    running_process->state = PROCESS_ZOMBIE;
    process_schedule(true, NULL);

}

/// Internal function that is implicitly called if the main function 
/// returns without calling exit. A process function returns an 
/// integer which will be placed in 'eax', we set this value to 
/// process' exit code before actually exiting.
static void process_implicit_exit(void) {
    // We need to ensure that eax is not overwritten by the prelude.
	__asm__ __volatile__("mov %%eax, %0" : "=r" (running_process->exit_code) :: "eax");
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
    process->state = PROCESS_AVAILABLE;

    return process;

}

void process_idle(process_entry_t entry, size_t stack_size, void *arg) {
    
    running_process = process_alloc(entry, stack_size, "idle", arg);

    // No parent/child.
    running_process->parent = NULL;
    running_process->child = NULL;
    running_process->sibling = NULL;

    // Insert process in right scheduler ring.
    running_process->priority = 0;
    process_sched_ring_insert(running_process);

    // TODO: Start preemptive scheduler.
    // [...]

    // Switch to the next process while throwing the dummy context,
    // because we'll never return to the kernel's execution.
    running_process->state = PROCESS_ACTIVE;
    process_context_switch(&dummy_context, &running_process->context);

}

pid_t process_start(process_entry_t entry, size_t stack_size, int priority, const char *name, void *arg) {
    
    // assert(running_process)
    // assert(priority < MAX_PRIO)

    struct process *process = process_alloc(entry, stack_size, name, arg);

    process->parent = running_process;
    process->child = NULL;

    // Insert the next process in the child linked list of the parent.
    process->sibling = running_process->child;
    running_process->child = process;

    // Insert in the right scheduler ring.
    process->priority = priority;
    process_sched_ring_insert(process);

    // If the new priority is higher than the current, we directly
    // interrupt the execution of the current process and switch to
    // this scheduler ring.
    if (process->priority > running_process->priority) {
        process_schedule(false, process);
    }

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

// TODO: Remove this as it's cooperative multi-tasking and will not be relevant soon.
void schedule() {
    process_schedule(false, NULL);
}
