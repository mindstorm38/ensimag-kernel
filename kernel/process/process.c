#include "process.h"
#include "segment.h"
#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"

#include "internals.h"
#include "memory.h"
#include "cpu.h"
#include "pit.h"
#include "syscall.h"

#include "../debug/user_stack_mem.h"


// TODO: Create a stack overflow detection with a sentinel at bottom
// of the stack, will be easier to debug.


/// Internal function to kill the given process. The given process 
/// must not be already a zombie.
static void process_internal_kill(struct process *process, int code, bool wake_parent) {

#if PROCESS_DEBUG
    printf("[%s] process_internal_kill(%s, %d, %d)\n", process_active->name, process->name, code, wake_parent);
#endif

    // Next process value, only used if the process is is active and
    // need to advance scheduling.
    struct process *next_process = NULL;
    
    struct process *parent = process->parent;
    if (wake_parent && parent != NULL) {
        if (parent->state == PROCESS_WAIT_CHILD) {
            pid_t parent_wait_pid = parent->wait_child.child_pid;
            if (parent_wait_pid < 0 || parent_wait_pid == process->pid) {

                // We need to reactivate the parent.
                parent->state = PROCESS_SCHED;
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

    // Note that we'll never enter this function if the parent state
    // is zombie, because any killed process sees its children
    // instantly killed before the parent will become ZOMBIE.

    if (process->state == PROCESS_SCHED) {

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
    } else if (process->state == PROCESS_WAIT_QUEUE) {
        // If the process is waiting queue, remove it from its queue.
        process_queue_kill_process(process);
    } else if (process->state == PROCESS_WAIT_CONS_READ) {
        // If the process is waiting for a console read, remove it.
        process_cons_read_kill_process(process);
    } else if (process->state == PROCESS_WAIT_CHILD) {
        // Process is waiting for a child, we do nothing because it
        // is not in any linked list.
    } else {
        panic("process_internal_kill(...): unsupported state when killing: %d\n", process->state);
    }

    // Here we want to free all child processes because they will never 
    // be awaited, so we can free all child processes.
    struct process *child_process = process->child;
    while (child_process != NULL) {

        if (child_process->state != PROCESS_ZOMBIE) {
            // Not waking-up parent because we are killing it.
            process_internal_kill(child_process, 0, false);
        }

        process_free(child_process);
        child_process = child_process->sibling;

    }
    process->child = NULL;

    // Process is zombie until waited for by parent.
    process->state = PROCESS_ZOMBIE;
    process->zombie.exit_code = code;

    // If killed process is the active one, schedule next one.
    if (process == process_active) {
        process_sched_advance(next_process);
    }

}

static void process_pit_handler(uint32_t clock) {
    process_sched_pit_handler(clock);
    process_time_pit_handler(clock);
}

void process_idle(process_entry_t entry, size_t stack_size, void *arg) {
    
    process_active = process_alloc(entry, stack_size, 0, "idle", arg);

    // No parent/child.
    process_active->parent = NULL;
    process_active->child = NULL;
    process_active->sibling = NULL;

    // Start preemptive scheduler.
    pit_set_handler(process_pit_handler);

    // Manually switch to the idle process context.
    process_context_switch(NULL, process_active);

}

pid_t process_start(process_entry_t entry, size_t stack_size, int priority, const char *name, void *arg) {
    
    // Invalid, function not called from a function.
    if (process_active == NULL)
        return -1;

    if (priority < 0 || priority >= PROCESS_MAX_PRIORITY)
        return -1;

    if (!process_check_user_ptr(name))
        return -1;
    
    struct process *process = process_alloc(entry, stack_size, priority, name, arg);
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
    printf("[%s] process_start(...) prio: %d, current prio: %d\n", process_active->name, process->priority, process_active->priority);
#endif

    // If the new priority is higher than the current, we directly
    // interrupt the execution of the current process and switch to
    // this scheduler ring.
    if (process->priority > process_active->priority) {
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

    if (process_active->pid == 0) {
        panic("Idle has exited, shutting down...");
    }

    process_internal_kill(process_active, code, true);
    // Never reached.
    while (1);

}

int32_t process_pid(void) {
    return process_active->pid;
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
    
    int prev_priority = process->priority;
    if (priority == prev_priority)
        return prev_priority; // No change to do.

    if (process->state == PROCESS_SCHED) {
        // The process is scheduled, call the scheduler.
        process_sched_set_priority(process, priority);
    } else if (process->state == PROCESS_WAIT_QUEUE) {
        // The process is waiting for a queue message, changing 
        // priority is a bit special here.
        process_queue_set_priority(process, priority);
    } else {
        // Other wait states doesn't require special priority handling.
        process->priority = priority;
    }

    return prev_priority;

}

pid_t process_wait(pid_t pid, int *exit_code) {

    struct process *child = process_active->child;
    if (child == NULL)
        return -1;
    
    if (exit_code != NULL && !process_check_user_ptr(exit_code))
        return -1;
    
#if PROCESS_DEBUG
    printf("[%s] process_wait(%d)\n", process_active->name, pid);
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
        printf("[%s] process_wait(...): waiting\n", process_active->name);
#endif

        struct process *next_process = process_sched_ring_remove(process_active);

        // Here, targetted child(ren) are not zombie: pause this process.
        process_active->state = PROCESS_WAIT_CHILD;
        process_active->wait_child.child_pid = pid;

        process_sched_advance(next_process);

        // Here we resume and the child should've set its pointer.
        child = process_active->sched.new_zombie_child;

#if PROCESS_DEBUG
        printf("[%s] process_wait(...): resume from %s\n", process_active->name, child->name);
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
        process_internal_kill(process, 0, true);
        return 0;

    }

}

int process_name(pid_t pid, char *dst, int count) {

    if (dst != NULL && count < 0)
        return -1;

    struct process *process = process_from_pid(pid);
    if (process == NULL)
        return -1;
    
    int name_len = strlen(process->name) + 1; // count 0
    strncpy(dst, process->name, count <= name_len ? count : name_len);
    
    return name_len;

}

int process_children(pid_t pid, pid_t *children_pids, int count) {

    if (children_pids != NULL && count < 0)
        return -1;

    if (children_pids != NULL && !process_check_user_ptr(children_pids))
        return -1;

    struct process *process = process_from_pid(pid);
    if (process == NULL)
        return -1;
    
    size_t total_count = 0;

    struct process *children = process->child;
    while (children != NULL) {

        if (count > 0 && children_pids != NULL) {
            children_pids[total_count] = children->pid;
            count--;
        }

        total_count++;
        children = children->sibling;
        
    }

    return total_count;

}

int process_state(pid_t pid) {

    struct process *process = process_from_pid(pid);
    if (process == NULL)
        return -1;
    
    return process_active == process ? 0 : (int) process->state + 1;

}


void process_wait_clock(uint32_t clock) {

#if PROCESS_DEBUG
    printf("[%s] process_wait_clock(%d)\n", process_active->name, clock);
#endif

    if (clock <= pit_clock_get()) {
        // The target clock is already passed, just advance.
        // FIXME: Do nothing? Or act like a 'yield'.
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


void process_debug(struct process *process) {

    printf("== [ PROCESS %d ] ==\n", process->pid);
    printf("NAME:    %s\n", process->name);
    printf("STATE:   %d\n", process->state);
    // printf("CONTEXT:\n");
    // printf(" EBX: 0x%08X\n", process->context.ebx);
    // printf(" ESP: 0x%08X\n", process->context.esp);
    // printf(" EBP: 0x%08X\n", process->context.ebp);
    // printf(" ESI: 0x%08X\n", process->context.esi);
    // printf(" EDI: 0x%08X\n", process->context.edi);

    // printf("STACK: %p -> %p\n", process->stack, process->stack + process->stack_size);
    // for (uint32_t *sp = (uint32_t *) (process->stack + process->stack_size - 1); (uint32_t) sp >= process->context.esp; sp--) {
    //     printf("%08X ", *sp);
    // }
    // printf("\n");

}


extern char user_start;
extern char user_end;

bool process_check_user_ptr(const void *ptr) {
    return (void *) &user_start <= ptr && ptr < (void *) &user_end;
}
