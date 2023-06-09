#include "process_internals.h"


static struct process *clock_wait_head = NULL;


void process_time_queue_add(struct process *process) {
    
    process->sched_prev = NULL;
    process->sched_next = clock_wait_head;
    
    if (clock_wait_head != NULL)
        clock_wait_head->sched_prev = process;
    
    clock_wait_head = process;

}

void process_time_queue_remove(struct process *process) {
    (void) process;
    // TODO:
}

void process_time_pit_handler(uint32_t clock) {
    (void) clock;
}
