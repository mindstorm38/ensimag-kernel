#include "internals.h"

#include "stdio.h"
#include "cpu.h"


/// Important to be initialize to all zero, we have a pointer to the
/// first process of the ring for each priority.
///
/// All processes in these rings are in `PROCESS_SCHED_*` states.
struct process *process_sched_rings[PROCESS_MAX_PRIORITY] = {0};

/// The currently active process, this process' priority is considered
/// to be the highest available in the scheduler rings: no highest 
/// priority process can be found. 
struct process *process_active = NULL;


void process_sched_ring_insert(struct process *process) {

    if (process->state != PROCESS_SCHED) {
        panic("process_sched_ring_insert(...): process->state must be SCHED");
    }

#if PROCESS_DEBUG
    if (process_active) {
        printf("[%s] process_sched_ring_insert(%s)\n", process_active->name, process->name);
    } else {
        printf("process_sched_ring_insert(%s)\n", process->name);
    }
#endif

    // Get the first process in this ring.
    struct process *first_process = process_sched_rings[process->priority];

    if (first_process) {

        // Insert our process just before first process.
        process->sched.next = first_process;
        process->sched.prev = first_process->sched.prev;

        // Update links.
        process->sched.next->sched.prev = process;
        process->sched.prev->sched.next = process;
        
    } else {
        // It's the first process, it's its own ring.
        process->sched.next = process;
        process->sched.prev = process;
    }

    process_sched_rings[process->priority] = process;

}

struct process *process_sched_ring_remove(struct process *process) {

#if PROCESS_DEBUG
    printf("[%s] process_sched_ring_remove(%s)\n", process_active->name, process->name);
#endif

    if (process->state != PROCESS_SCHED) {
        panic("process_sched_ring_remove(...): process->state must be SCHED");
    }

    // Remove the running process from its own ring.
    struct process *prev_process = process->sched.prev;
    struct process *next_process = process->sched.next;

    // Process is already removed from its ring.
    if (prev_process == NULL || next_process == NULL) {
        return NULL;
    }

    // If the process is it's own next process, it means that it was
    // alone in its ring. So we set all to NULL.
    if (next_process == process) {
        next_process = NULL;
        prev_process = NULL;
    } else {
        prev_process->sched.next = next_process;
        next_process->sched.prev = prev_process;
    }

    // Update f process of the ring.
    struct process **first_process_ptr = &process_sched_rings[process->priority];
    if (*first_process_ptr == process) {
        *first_process_ptr = next_process;
    }

    // Nullify its pointers, anyway, the state should be changed 
    // afterward.
    process->sched.prev = NULL;
    process->sched.next = NULL;

#if PROCESS_DEBUG
    if (next_process != NULL) {
        printf("[%s] process_sched_ring_remove(%s) -> %s\n", process_active->name, process->name, next_process->name);
    } else {
        printf("[%s] process_sched_ring_remove(%s) -> NULL\n", process_active->name, process->name);
    }
#endif

    return next_process;

}

struct process *process_sched_ring_find(int max_priority) {

    // printf("process_sched_ring_find(%d)\n", max_priority);

    for (int priority = max_priority - 1; priority >= 0; priority--) {
        if (process_sched_rings[priority] != NULL) {
            // printf("[%d] process: %s\n", priority, process_sched_rings[priority]->name);
            return process_sched_rings[priority];
        }
    }

    // Should not happen (because we have idle process at prio 0).
    panic("process_sched_ring_find(%d): failed to find a process, this should not happen because idle should be present\n", max_priority);

}

void process_sched_advance(struct process *next_process) {

#if PROCESS_DEBUG
    if (next_process != NULL) {
        printf("[%s] process_sched_advance(%s)\n", process_active->name, next_process->name);
    } else {
        printf("[%s] process_sched_advance(NULL)\n", process_active->name);
    }
#endif

    // if (prev_process->state == PROCESS_SCHED_ACTIVE) {
    //     panic("process_sched_advance(...): process_active->state cannot be SCHED_ACTIVE\n");
    // }

    // The active process should remain scheduled, we can search for
    // the next process in its ring.
    if (next_process == NULL && process_active->state == PROCESS_SCHED) {
        next_process = process_active->sched.next;
    }

    // Still no process? Go lower in schedule ring.
    if (next_process == NULL) {
        
        next_process = process_sched_ring_find(process_active->priority);

#if PROCESS_DEBUG
        printf("[%s] process_sched_advance(...): ring find: %s\n", process_active->name, next_process->name);
#endif

    }

    next_process->state = PROCESS_SCHED;

    // Do not context switch if the same process loops over.
    if (process_active == next_process) {
        // Do nothing
    } else {
        struct process *prev_process = process_active;
        process_active = next_process;
        process_context_switch(prev_process, next_process);
    }

}

void process_sched_set_priority(struct process *process, int new_priority) {

    if (process->state != PROCESS_SCHED) {
        panic("process_sched_set_priority(...): processs->state is not SCHED\n");
    }

    // Remove the process from its current ring.
    struct process *next_process = process_sched_ring_remove(process);
    int prev_priority = process->priority;
    process->priority = new_priority;
    // Insert in its new ring.
    process_sched_ring_insert(process);

    if (new_priority < prev_priority) {

        // New priority is less than previous one.
        if (process_active == process) {

            // If we are the current process, we need to find another
            // process to run because we now have a lower priority.
            if (next_process == NULL) {

                // Here we need to find a process on a new ring 
                // because our ring is now empty.
                next_process = process_sched_ring_find(prev_priority);
                if (next_process == process)
                    return;

            }

        } else {
            // Not the current process, our process must have a
            // priority lower to the active process.
            return;
        }

    } else {

        // New priority is higher than current one.
        if (process_active == process) {
            // We were already the highest priority process, no need
            // to context switch.
            return;
        } else if (new_priority > process_active->priority) {
            // New priority is higher than current one, switch to our
            // process.
            next_process = process;
        } else {
            // New priority doesn't require to schedule the process.
            return;
        }

    }

    if (next_process != NULL && next_process != process) {
        process_sched_advance(next_process);
    }

}

void process_sched_pit_handler(uint32_t clock) {

    (void) clock;

    // Interruptions are disabled here, don't need to cli.    
    process_sched_advance(NULL);

}
