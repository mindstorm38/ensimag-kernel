#include "process.h"
#include "process_internals.h"


/// Head of the all-processes linked list head.
static struct process *overall_head = NULL;
static pid_t pid_counter = 0;


void process_overall_add(struct process *process) {

    process->overall_next = overall_head;
    process->overall_prev = NULL;
    
    if (overall_head != NULL)
        overall_head->overall_prev = process;

    process->pid = pid_counter++;

    overall_head = process;

}

void process_overall_remove(struct process *process) {

    if (process->overall_next)
        process->overall_next->overall_prev = process->overall_prev;
    
    if (process->overall_prev)
        process->overall_prev->overall_next = process->overall_next;

    // Our process is the list's head.
    if (process->overall_prev == NULL)
        overall_head = process->overall_next;

}

struct process *process_from_pid(pid_t pid) {
    
    struct process *process = overall_head;
    while (process != NULL) {
        if (process->pid == pid)
            return process;
        process = process->overall_next;
    }

    return NULL;

}
