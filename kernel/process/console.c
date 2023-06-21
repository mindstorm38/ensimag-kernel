/// Console read on process.

#include "internals.h"


size_t process_wait_cons_read(char *dst, size_t len) {

    (void) dst;
    (void) len;
    
    struct process *next_process = process_sched_ring_remove(process_active);

    process_active->state = PROCESS_WAIT_CONS_READ;

    process_sched_advance(next_process);

    return 0;

}
