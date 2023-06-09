#include "process_internals.h"

#include "stdio.h"
#include "cpu.h"


/// Important to be initialize to all zero, we have a pointer to the
/// first process of the ring for each priority.
struct process *process_sched_rings[PROCESS_MAX_PRIORITY] = {0};

/// The currently active process, this process' priority is considered
/// to be the highest available in the scheduler rings: no highest 
/// priority process can be found. 
struct process *process_active = NULL;


void process_sched_ring_insert(struct process *process) {

    // Get the first process in this ring.
    struct process *first_process = process_sched_rings[process->priority];
    
    if (first_process) {

        // Insert our process just before first process.
        process->sched_next = first_process;
        process->sched_prev = first_process->sched_prev;

        // Update links.
        process->sched_next->sched_prev = process;
        process->sched_prev->sched_next = process;
        
    } else {
        // It's the first process, it's its own ring.
        process->sched_next = process;
        process->sched_prev = process;
    }

    process_sched_rings[process->priority] = process;

}

struct process *process_sched_ring_remove(struct process *process) {

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

struct process *process_sched_ring_find(int max_priority) {

    for (int priority = max_priority - 1; priority >= 0; priority--) {
        if (process_sched_rings[priority] != NULL) {
            return process_sched_rings[priority];
        }
    }

    // Should not happen, TODO: panic?
    return NULL;

}

void process_sched_advance(struct process *next_process, bool ring_remove) {

    struct process *prev_process = process_active;

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

    next_process->state = PROCESS_ACTIVE;

    // Do not context switch if the same process loops over.
    if (prev_process == next_process) {
        // Do nothing
    } else {
        
        process_active = next_process;
        process_context_switch(&prev_process->context, &next_process->context);
        
    }

}

int process_sched_set_priority(struct process *process, int new_priority) {

    int prev_priority = process->priority;
    if (prev_priority == new_priority)
        return prev_priority;

    // Remove the process from its current ring.
    struct process *next_process = process_sched_ring_remove(process);

    // Insert the process into its new ring.
    process->priority = new_priority;
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
                if (next_process == process) {
                    // Our process is also the new highest priority one.
                    next_process = NULL;
                }
            }
        } else {
            // Not the current process, our process must have a
            // priority lower to the active process.
            next_process = NULL; // No need to context switch.
        }

    } else {

        // New priority is higher than current one.
        if (process_active == process) {
            // We were already the highest priority process, no need
            // to context switch.
            next_process = NULL;
        } else if (new_priority > process_active->priority) {
            // New priority is higher than current one, switch to our
            // process.
            next_process = process;
        }

    }

    if (next_process != NULL && next_process != process) {
        process_sched_advance(next_process, false);
    }

    return prev_priority;

}

void process_sched_pit_handler(uint32_t clock) {

    (void) clock;

    // Interruptions are disabled here, don't need to cli.    
    process_active->state = PROCESS_AVAILABLE;
    process_sched_advance(NULL, false);

}
