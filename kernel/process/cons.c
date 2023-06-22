/// Console read on process.

#include "internals.h"
#include "cons.h"

#include "stdio.h"


static struct process *cons_read_wait_head = NULL;


/// Internal function called by the console driver when enough data
/// can be read. We wake all process so they will 
static void process_wait_cons_read_wake(void) {

    struct process *process_it = cons_read_wait_head;
    struct process *highest_process = NULL;

    while (process_it != NULL) {
        if (highest_process == NULL || highest_process->priority < process_it->priority) {
            highest_process = process_it;
        }
        struct process *next_process = process_it->wait_cons_read.next;
        process_it->state = PROCESS_SCHED;
        process_sched_ring_insert(process_it);
        process_it = next_process;
    }

    cons_read_wait_head = NULL;

    if (highest_process != NULL && highest_process->priority > process_active->priority) {
        process_sched_advance(highest_process);
    }

}

size_t process_wait_cons_read(char *dst, size_t len) {

    if (len == 0)
        return 0;

    while (1) {

        size_t read_len = len;
        if (cons_try_read(dst, &read_len, process_wait_cons_read_wake))
            return read_len;
        
        struct process *next_process = process_sched_ring_remove(process_active);
        process_active->state = PROCESS_WAIT_CONS_READ;
        process_active->wait_cons_read.next = cons_read_wait_head;
        cons_read_wait_head = process_active;

        process_sched_advance(next_process);

    }

}

void process_cons_read_kill_process(struct process *process) {

    // Remove the process from its wait list.
    struct process **process_it = &cons_read_wait_head;
    while (*process_it != NULL) {
        if (*process_it == process) {
            *process_it = process->wait_cons_read.next;
            break;
        }
        process_it = &(*process_it)->wait_cons_read.next;
    }

}
