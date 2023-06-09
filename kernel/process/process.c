#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"

#include "process_internals.h"
#include "memory.h"
#include "cpu.h"
#include "pit.h"


static pid_t pid_counter = 0;
// This context is never used at restoration.
static struct process_context dummy_context;


/// Internal function that actually exits and schedule next process.
__attribute__((noreturn))
static void process_internal_exit(void) {

    // TODO: Free stack/process? Maybe in waitpid...

    // Process is zombie until waited for by parent.
    process_active->state = PROCESS_ZOMBIE;
    process_sched_advance(NULL, true);

    // Never reached.
    while (1);

}

/// Internal function that is implicitly called if the main function 
/// returns without calling exit. A process function returns an 
/// integer which will be placed in 'eax', we set this value to 
/// process' exit code before actually exiting.
static void process_implicit_exit(void) {
    // We need to ensure that eax is not overwritten by the prelude.
    // EAX set in clobbers so it's not overwritten while doing this.
	__asm__ __volatile__("mov %%eax, %0" : "=r" (process_active->exit_code) :: "eax");
    process_internal_exit();
}

/// Internal function that allocate a process given. Callers of this
/// function ('process_idle' and 'process_start' only) need to 
/// initialize remaining fields (parent/child/sibling/state).
/// 
/// Interrupts must be disabled before calling this function.
static struct process *process_alloc(process_entry_t entry, size_t stack_size, int priority, const char *name, void *arg) {

    // TODO: Check allocation errors.
    struct process *process = kalloc(sizeof(struct process));
    if (process == NULL)
        return NULL;

    void *stack = kalloc(stack_size);
    if (stack == NULL) {
        kfree(stack);
        return NULL;
    }

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

    // Overall list.
    process_overall_add(process);

    // Priority and scheduler ring.
    process->priority = priority;
    process_sched_ring_insert(process);

    return process;

}

void process_idle(process_entry_t entry, size_t stack_size, void *arg) {
    
    process_active = process_alloc(entry, stack_size, 0, "idle", arg);
    process_active->state = PROCESS_ACTIVE;

    // No parent/child.
    process_active->parent = NULL;
    process_active->child = NULL;
    process_active->sibling = NULL;

    // Start preemptive scheduler.
    pit_set_handler(process_sched_pit_handler);

    // Manually switch to the idle process context.
    process_context_switch(&dummy_context, &process_active->context);

}

pid_t process_start(process_entry_t entry, size_t stack_size, int priority, const char *name, void *arg) {
    
    // assert(running_process)
    // assert(priority < MAX_PRIO)

    struct process *process = process_alloc(entry, stack_size, priority, name, arg);
    process->state = PROCESS_AVAILABLE;

    // Parent is current process.
    process->parent = process_active;
    process->child = NULL;

    // Insert the next process in the child linked list of the parent.
    process->sibling = process_active->child;
    process_active->child = process;

    // If the new priority is higher than the current, we directly
    // interrupt the execution of the current process and switch to
    // this scheduler ring.
    if (process->priority > process_active->priority) {
        process_sched_advance(process, false);
    }

    return process->pid;

}

void process_exit(int code) {
    process_active->exit_code = code;
    process_internal_exit();
}

int32_t process_pid(void) {
    return process_active->pid;
}

char *process_name(void) {
    return process_active->name;
}

void process_wait(uint32_t clock) {

    (void) clock;

    // process_active->state = PROCESS_WAIT_TIME;
    // // TODO: Add the running process to a queue of time.
    // process_sched_advance(NULL, true);

}

int process_priority(pid_t pid) {

    struct process *process = process_from_pid(pid);

    if (process == NULL)
        return -1;
    
    return process->priority;

}

int process_set_priority(pid_t pid, int priority) {

    struct process *process = process_from_pid(pid);

    if (process != NULL)
        return process_sched_set_priority(process, priority);
    else
        return -1;

}


void process_debug(void) {

    printf("process %p\n", process_active);
    printf("- %s (%d)\n", process_active->name, process_active->pid);

    if (process_active->sched_prev)
        printf("- sched_prev: %s (%d)\n", process_active->sched_prev->name, process_active->sched_prev->pid);
    if (process_active->sched_next)
        printf("- sched_next: %s (%d)\n", process_active->sched_next->name, process_active->sched_next->pid);

    if (process_active->parent)
        printf("- parent: %s (%d)\n", process_active->parent->name, process_active->parent->pid);

    printf("- children:");
    struct process *child = process_active->child;
    while (child) {
        printf(" %s (%d)", child->name, child->pid);
        child = child->sibling;
    }
    printf("\n");

    printf("- esp: %p\n", (void *) process_active->context.esp);

}
