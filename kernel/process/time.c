#include "internals.h"
#include "stdio.h"


static struct process *clock_wait_head = NULL;


void process_time_queue_add(struct process *process) {

    if (process->state != PROCESS_WAIT_TIME) {
        panic("process_time_queue_add(...): process->state is not WAIT_TIME\n");
    }

    uint32_t target_clock = process->wait_time.target_clock;

    // Here we insert the process in the right order. Our linked list
    // is ordered by target clock time.

    // TODO: Order by process priority?
    
    struct process **process_ptr = &clock_wait_head;
    while (*process_ptr != NULL) {
        if (target_clock <= (*process_ptr)->wait_time.target_clock) {
            // Our process has lower or equal target clock, we insert
            // it in place.
            break;
        }
    }

    // Insert process at the given pointer.
    process->wait_time.next = *process_ptr;
    *process_ptr = process;

}

void process_time_queue_remove(struct process *process) {
    
    if (process->state != PROCESS_WAIT_TIME) {
        panic("process_time_queue_remove(...): process->state is not WAIT_TIME\n");
    }

    struct process **process_ptr = &clock_wait_head;
    while (*process_ptr != NULL) {
        if (*process_ptr == process) {
            *process_ptr = process->wait_time.next;
            break;
        }
    }

}

void process_time_pit_handler(uint32_t clock) {

    // printf("pit clock: %d\n", clock);

    struct process *highest_process = NULL;

    struct process *process = clock_wait_head;
    while (process != NULL && process->wait_time.target_clock <= clock) {

#if PROCESS_DEBUG
        if (process->state != PROCESS_WAIT_TIME) {
            panic("[%s] process_time_pit_handler(...): process %s reached target clock but is in state %d\n", process_active->name, process->name, process->state);
        }
#endif

        // Find the highest priority between all woken up process.
        if (highest_process == NULL || highest_process->priority < process->priority) {
            highest_process = process;
        }

        struct process *next_process = process->wait_time.next;

        // Re-schedule the process that reached target clock.
        process->state = PROCESS_SCHED_AVAILABLE;
        process_sched_ring_insert(process);

        process = next_process;
        
    }

    // New head is the next process in time.
    clock_wait_head = process;

    // If the new highest priority process has greater priority than
    // currently running process, we schedule our new process.
    if (highest_process != NULL && highest_process->priority > process_active->priority) {
        process_active->state = PROCESS_SCHED_AVAILABLE;
        process_sched_advance(highest_process);
    }

}
