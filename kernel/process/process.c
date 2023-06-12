#include "process.h"
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


/// Internal function to kill the given process. The given process 
/// must not be already a zombie.
static void process_internal_kill(struct process *process, int code) {

    struct process *next_process = NULL;
    struct process *parent = process->parent;

#if PROCESS_DEBUG
    printf("[%s] process_internal_kill(%s, %d)\n", process_active->name, process->name, code);
#endif

    if (parent != NULL) {
        if (parent->state == PROCESS_WAIT_CHILD) {
            pid_t parent_wait_pid = parent->wait_child.child_pid;
            if (parent_wait_pid < 0 || parent_wait_pid == process->pid) {

                // We need to reactivate the parent.
                parent->state = PROCESS_SCHED_AVAILABLE;
                parent->sched.new_zombie_child = process;
                // Re-insert the parent in its ring.
                process_sched_ring_insert(parent);

                // If parent's priority is higher that currently 
                // exiting process, schedule the parent: 
                if (parent->priority > process->priority) {
                    next_process = parent;
                }

            }
        }
    }

    // TODO: If parent state is ZOMBIE, or is null, just free the 
    // process because it will never be waited for.

    if (process->state == PROCESS_SCHED_ACTIVE || process->state == PROCESS_SCHED_AVAILABLE) {

        // If the process was active or available, remove it from scheduler.
        struct process *next_ring_process = process_sched_ring_remove(process);

        // If no next process is already set (higher priority), use the 
        // next ring one.
        if (next_process == NULL) {
            next_process = next_ring_process;
        }

    } else if (process->state == PROCESS_WAIT_TIME) {

        // If the process was waiting time, remove it from the queue.
        process_time_queue_remove(process);

    } else {
        panic("process_internal_kill(...): unsupported state when exiting: %d\n", process->state);
    }

    // Process is zombie until waited for by parent.
    process->state = PROCESS_ZOMBIE;
    process->zombie.exit_code = code;

    // If killed process is the active one, schedule next one.
    if (process == process_active) {
        process_sched_advance(next_process);
    }

}

/// Internal function that allocate a process given. Callers of this
/// function ('process_idle' and 'process_start' only) need to 
/// initialize remaining fields (parent/child/sibling/state).
/// 
/// Interrupts must be disabled before calling this function.
static struct process *process_alloc(process_entry_t entry, size_t stack_size, int priority, const char *name, void *arg, enum process_state state) {

    struct process *process = kalloc(sizeof(struct process));
    if (process == NULL)
        return NULL;

    // FIXME: This value need to be adjusted for function called at 
    // exit or in interrupts, it's not easy to get it right, for 
    // example using printf in those function requires 128 more words.
    size_t more_size = sizeof(size_t) * 32;
    size_t prelude_size = sizeof(size_t) * 3;

    // Check overflow while adding prelude size.
    if (__builtin_add_overflow(stack_size, prelude_size + more_size, &stack_size)) {
        kfree(process);
        return NULL;
    }

    void *stack = kalloc(stack_size);
    if (stack == NULL) {
        kfree(process);
        return NULL;
    }

    // Initialize stack...
    process->stack = stack;

    void *stack_top = stack + stack_size;
    size_t *stack_ptr = stack_top - prelude_size;
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
    process->state = state;
    process_sched_ring_insert(process);

    return process;

}

/// Internal function to free the given process. Caller must ensure
/// that this process is present in the overall list. The process
/// should not be the active one.
/// 
/// This is typically used when the process has been a zombie and is
/// waited for by its parent, or if its parent dies.
static void process_free(struct process *process) {

    if (process->state != PROCESS_ZOMBIE) {
        panic("process_free(...): process->state not ZOMBIE");
    }

    // Remove the process from overall list.
    process_overall_remove(process);

    // Remove it from its parent/sibling relationship.
    struct process *parent = process->parent;
    if (parent != NULL) {

        struct process **child_ptr = &parent->child;
        while (*child_ptr != NULL) {
            if (*child_ptr == process) {
                // We found the child to remove, set the parent ptr to
                // its sibling (which may be null if no sibling).
                *child_ptr = process->sibling;
                break;
            }
            child_ptr = &(*child_ptr)->sibling;
        }

    }

    // We need to null-ptr the child's parent, to avoid storing an
    // invalid pointer.
    struct process *child = process->child;
    while (child != NULL) {
        child->parent = NULL;
        child = child->sibling;
    }

    // Free resources.
    kfree(process->stack);
    kfree(process);

}

static void process_pit_handler(uint32_t clock) {
    process_sched_pit_handler(clock);
    process_time_pit_handler(clock);
}

void process_idle(process_entry_t entry, size_t stack_size, void *arg) {
    
    process_active = process_alloc(entry, stack_size, 0, "idle", arg, PROCESS_SCHED_ACTIVE);

    // No parent/child.
    process_active->parent = NULL;
    process_active->child = NULL;
    process_active->sibling = NULL;

    // Start preemptive scheduler.
    pit_set_handler(process_pit_handler);

    // Manually switch to the idle process context.
    process_context_switch(&dummy_context, &process_active->context);

}

pid_t process_start(process_entry_t entry, size_t stack_size, int priority, const char *name, void *arg) {
    
    // Invalid, function not called from a function.
    if (process_active == NULL)
        return -1;

    if (priority < 0 || priority >= PROCESS_MAX_PRIORITY)
        return -1;
    
    struct process *process = process_alloc(entry, stack_size, priority, name, arg, PROCESS_SCHED_AVAILABLE);
    if (process == NULL)
        return -1; // Allocation error.
    
    // Parent is current process.
    process->parent = process_active;
    process->child = NULL;

    // Insert the next process in the child linked list of the parent.
    process->sibling = process_active->child;
    process_active->child = process;
    
#if PROCESS_DEBUG
    printf("[%s] process_start(...) -> %s\n", process_active->name, name);
#endif

    // If the new priority is higher than the current, we directly
    // interrupt the execution of the current process and switch to
    // this scheduler ring.
    if (process->priority > process_active->priority) {
        process_active->state = PROCESS_SCHED_AVAILABLE;
        process_sched_advance(process);
    }

    return process->pid;

}

void process_exit(int code) {
    process_internal_exit(code);
}

/// Internal function that actually exits from active process.
void process_internal_exit(int code) {

#if PROCESS_DEBUG
    printf("[%s] process_internal_exit(%d)\n", process_active->name, code);
#endif

    process_internal_kill(process_active, code);
    // Never reached.
    while (1);

}

int32_t process_pid(void) {
    return process_active->pid;
}

char *process_name(void) {
    return process_active->name;
}

int process_priority(pid_t pid) {

    struct process *process = process_from_pid(pid);

    if (process == NULL)
        return -1;
    
    return process->priority;

}

int process_set_priority(pid_t pid, int priority) {

    if (priority < 0 || priority >= PROCESS_MAX_PRIORITY)
        return -1;

    struct process *process = process_from_pid(pid);
    if (process == NULL || process->state == PROCESS_ZOMBIE)
        return -1;

    return process_sched_set_priority(process, priority);

}

pid_t process_wait_pid(pid_t pid, int *exit_code) {

    struct process *child = process_active->child;
    if (child == NULL)
        return -1;
    
#if PROCESS_DEBUG
    printf("[%s] process_wait_pid(%d)\n", process_active->name, pid);
#endif
    
    while (child != NULL) {
        if (pid < 0 && child->state == PROCESS_ZOMBIE) {
            // If we search for any child, if we found a zombie we can
            // directly return and it will be removed below.
            break;
        } else if (pid == child->pid) {
            // If we search a particular child, we break and check it.
            break;
        }
        child = child->sibling;
    }

    // Pointer is null but we are searching for a particular 
    // process, so no child exists with this pid.
    if (child == NULL && pid >= 0)
        return -1;
    
    // If child == NULL must be pid < 0 (above condition).
    if (child == NULL || child->state != PROCESS_ZOMBIE) {

#if PROCESS_DEBUG
        printf("[%s] process_wait_pid(...): waiting\n", process_active->name);
#endif

        struct process *next_process = process_sched_ring_remove(process_active);

        // Here, targetted child(ren) are not zombie: pause this process.
        process_active->state = PROCESS_WAIT_CHILD;
        process_active->wait_child.child_pid = pid;

        process_sched_advance(next_process);

        // Here we resume and the child should've set its pointer.
        child = process_active->sched.new_zombie_child;

#if PROCESS_DEBUG
        printf("[%s] process_wait_pid(...): resume from %s\n", process_active->name, child->name);
#endif

    }

    // Here, child should not be null and its state should be ZOMBIE.
    if (exit_code != NULL)
        *exit_code = child->zombie.exit_code;
    
    process_free(child);

    return child->pid;

}

int process_kill(pid_t pid) {
    
    // Can't kill idle.
    if (pid == 0)
        return -1;

    if (pid == process_active->pid) {
        // Killing itself.
        process_internal_exit(0);
    } else {

        struct process *process = process_from_pid(pid);
        if (process == NULL || process->state == PROCESS_ZOMBIE)
            return -1;

        // Killing another process.
        process_internal_kill(process, 0);
        return 0;

    }

}

void process_wait_clock(uint32_t clock) {

#if PROCESS_DEBUG
    printf("[%s] process_wait_clock(%d)\n", process_active->name, clock);
#endif

    if (clock <= pit_clock()) {

        // The target clock is already passed, just advance.
        process_active->state = PROCESS_SCHED_AVAILABLE;
        process_sched_advance(NULL);

    } else {
        
        struct process *next_process = process_sched_ring_remove(process_active);

        process_active->state = PROCESS_WAIT_TIME;
        process_active->wait_time.target_clock = clock;

        // Add the process to the time queue and advanced scheduling while
        // removing it from the ring.
        process_time_queue_add(process_active);
        
        process_sched_advance(next_process);

    }

}


void process_debug(void) {

    printf("process %p\n", process_active);
    printf("- %s (%d)\n", process_active->name, process_active->pid);

    // if (process_active->sched_prev)
    //     printf("- sched_prev: %s (%d)\n", process_active->sched_prev->name, process_active->sched_prev->pid);
    // if (process_active->sched_next)
    //     printf("- sched_next: %s (%d)\n", process_active->sched_next->name, process_active->sched_next->pid);

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
