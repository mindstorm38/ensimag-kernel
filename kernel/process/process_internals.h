#ifndef __PROCESS_INTERNALS_H__
#define __PROCESS_INTERNALS_H__

#include "process.h"
#include "stdbool.h"
#include "stdint.h"


struct process;

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

union process_state_data {
    /// Valid when `PROCESS_ZOMBIE`, it's used for returning
    /// the exit code to the parent that waits for it.
    int zombie_exit_code;
    /// Valid when `PROCESS_WAIT_CHILD`.
    pid_t wait_child_pid;
    /// This value is valid in `PROCESS_ACTIVE` but just after being 
    /// resumed after `PROCESS_WAIT_CHILD` , it became invalid after 
    /// `process_wait_pid` returns.
    struct process *active_new_zombie_child;
    /// This value is valid in `PROCESS_WAIT_TIME` and contains the
    /// clock value that will wake up this process.
    uint32_t wait_time_clock;
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
    ///
    /// Note that when the process is in a wait state, and therefore
    /// not in the scheduler ring, this variable may be used for the
    /// wait linked list.
    struct process *sched_prev;
    /// Next process in the schedule ring for the current priority.
    ///
    /// Note that when the process is in a wait state, and therefore
    /// not in the scheduler ring, this variable may be used for the
    /// wait linked list.
    struct process *sched_next;
    /// Pointer to the parent process.
    struct process *parent;
    /// Head of the child linked list.
    struct process *child;
    /// Next child of the parent process's child linked list.
    struct process *sibling;
    /// State of the process.
    enum process_state state;
    /// Optional state data, depending on the state.
    union process_state_data state_data;
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
};


extern struct process *process_active;


/// Function defined in assembly (process_context.S).
void process_context_switch(struct process_context *prev_ctx, struct process_context *next_ctx);

void process_internal_exit(int code) __attribute__((noreturn));
/// Method defined in assembly to be sure that EAX don't get clobbered.
void process_implicit_exit(void) __attribute__((noreturn));

/// Internal function used to insert a process in its scheduler ring.
void process_sched_ring_insert(struct process *process);
/// Internal function used to remove a process from its scheduler ring.
/// Returns the next process after the removed one, might be null if
/// the ring is now empty.
///
/// If the process is not in a ring, nothing is done.
struct process *process_sched_ring_remove(struct process *process);
/// Internal function used to find a non-null schedule ring up to
/// and excluding the given priority. This should not return null
/// since at least ring 0 should contain idle process.
struct process *process_sched_ring_find(int max_priority);
/// A boolean parameter can be used to remove the previous process
/// from its scheduler ring.
///
/// The next process may optionally be specified, if not the case the
/// next process in the ring is used, if there is no more process in 
/// the ring, a lower-priority ring process is scheduled. Important:
/// this function doesn't search for higher priority.
///
/// The next process state is set to ACTIVE, and the previous process
/// state is untouched, so the caller must set it before calling this
/// function.
void process_sched_advance(struct process *next_process, bool ring_remove);
/// Change the scheduling priority of the given process, this function
/// automatically handles context switch if the next priority is
/// higher than the current process. Previous priority is returned.
///
/// The given process must not be a zombie.
int process_sched_set_priority(struct process *process, int new_priority);
/// Internal function that handle pit interrupts for scheduler.
void process_sched_pit_handler(uint32_t clock);

/// Add the process to the clock queue.
void process_time_queue_add(struct process *process);
/// Add the process to the clock queue.
void process_time_queue_remove(struct process *process);
/// Internal function that handle pit interrupts for clock.
void process_time_pit_handler(uint32_t clock);

/// Register the process in the overall linked list.
void process_overall_add(struct process *process);
/// Remove the process from the overall linked list.
void process_overall_remove(struct process *process);
/// Get a process from its PID.
struct process *process_from_pid(pid_t pid);

#endif
