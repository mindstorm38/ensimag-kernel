#ifndef __PROCESS_INTERNALS_H__
#define __PROCESS_INTERNALS_H__

#include "process.h"
#include "stdbool.h"
#include "stdint.h"


/// States that a process can take, used for scheduling.
enum process_state {
    PROCESS_ACTIVE,
    PROCESS_AVAILABLE,
    PROCESS_WAIT_QUEUE,
    PROCESS_WAIT_SEMAPHORE,
    PROCESS_WAIT_IO,
    PROCESS_WAIT_CHILD,
    PROCESS_WAIT_TIME,
    PROCESS_ZOMBIE,
};

/// Registers context of a process.
struct process_context {
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
};

/// The process structure.
struct process {
    /// Previous process in the overall linked list.
    struct process *overall_prev;
    /// Next process in the overall linked list.
    struct process *overall_next;
    /// Previous process in the schedule ring for the current priority.
    struct process *sched_prev;
    /// Next process in the schedule ring for the current priority.
    struct process *sched_next;
    /// Pointer to the parent process.
    struct process *parent;
    /// Head of the child linked list.
    struct process *child;
    /// Next child of the parent process's child linked list.
    struct process *sibling;
    /// State of the process.
    enum process_state state;
    /// Context saving registers that will be restored on context switch.
    struct process_context context;
    /// The process' stack.
    char *stack;
    /// Process id, its index in the internal process queue.
    pid_t pid;
    /// Name of the process.
    char name[PROCESS_NAME_MAX_SIZE];
    /// Scheduling priority, from 0 to PROCESS_MAX_PRIORITY excluded.
    int priority;
    /// Valid when the process is a zombie, it's used for returning
    /// the exit code to the parent that waits for it.
    int exit_code;
};


extern struct process *process_active;


/// Function defined in assembly (process_context.S).
///
/// Interrupts must be re-enable before calling this function.
void process_context_switch(struct process_context *prev_ctx, struct process_context *next_ctx);

/// Internal function used to insert a process in its scheduler ring.
///
/// Interrupts must be disabled while calling this function.
void process_sched_ring_insert(struct process *process);
/// Internal function used to remove a process from its scheduler ring.
/// Returns the next process after the removed one, might be null if
/// the ring is now empty.
///
/// Interrupts must be disabled while calling this function.
struct process *process_sched_ring_remove(struct process *process);
/// Internal function used to find a non-null schedule ring up to
/// and excluding the given priority. This should not return null
/// since at least ring 0 should contain idle process.
///
/// Interrupts must be disabled while calling this function.
struct process *process_sched_ring_find(int max_priority);
/// A boolean parameter can be used to remove the previous process
/// from its scheduler ring.
///
/// The next process may optionally be specified, if not the case the
/// next process in the ring is used, if there is no more process in 
/// the ring, a lower-priority ring process is scheduled.
///
/// The next process state is set to ACTIVE, and the previous process
/// state is untouched, so the caller must set it before calling this
/// function.
///
/// Interrupts must be disabled while calling this function.
/// This function may or not be called from an interrupt handler, in
/// any case this function re-enable the interrupts before switching.
void process_sched_advance(struct process *next_process, bool ring_remove);
/// Internal function that handle pit interrupts.
void process_sched_pit_handler(uint32_t clock);

/// Register the process in the overall linked list.
///
/// Interrupts must be disabled while calling this function.
void process_overall_add(struct process *process);
/// Remove the process from the overall linked list.
///
/// Interrupts must be disabled while calling this function.
void process_overall_remove(struct process *process);
/// Get a process from its PID.
///
/// Interrupts must be disabled while calling this function.
struct process *process_from_pid(pid_t pid);

#endif
