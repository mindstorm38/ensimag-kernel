#ifndef __PROCESS_INTERNALS_H__
#define __PROCESS_INTERNALS_H__

#include "process.h"
#include "stdbool.h"
#include "stdint.h"


#define PROCESS_DEBUG 0


struct process;

/// States that a process can take, used for scheduling.
enum process_state {
    /// The process is the currently executed one, it must be in the
    /// scheduler rings of its priority.
    PROCESS_SCHED_ACTIVE,
    /// The process is in the scheduler ring and is waiting to be
    /// executed.
    PROCESS_SCHED_AVAILABLE,
    PROCESS_WAIT_QUEUE,
    PROCESS_WAIT_SEMAPHORE,
    PROCESS_WAIT_IO,
    PROCESS_WAIT_CHILD,
    PROCESS_WAIT_TIME,
    PROCESS_ZOMBIE,
};

/// Scheduler-specific state for process that are in 
/// `PROCESS_SCHED_ACTIVE` or `PROCESS_SCHED_AVAILABLE`.
struct process_state_sched {
    /// Previous process in the schedule ring for the current priority.
    struct process *prev;
    /// Next process in the schedule ring for the current priority.
    struct process *next;
    /// Used when resuming from the `PROCESS_WAIT_CHILD`, it gives a
    /// pointer to the child that became a zombie.
    struct process *new_zombie_child;
};

/// For process that are in `PROCESS_WAIT_CHILD`.
struct process_state_wait_child {
    /// The pid that is waited for (negative for any child).
    pid_t child_pid;
};

struct process_state_wait_time {
    /// Target clock time at which the process should be rescheduled.
    uint32_t target_clock;
    /// Next process in the wait time linked list.
    struct process *next;
};

/// Zombie-specific state for process that are in `PROCESS_ZOMBIE`.
struct process_state_zombie {
    /// Exit code of the process.
    int exit_code;
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
    /// Pointer to the parent process.
    struct process *parent;
    /// Head of the child linked list.
    struct process *child;
    /// Next child of the parent process's child linked list.
    struct process *sibling;
    /// Context saving registers that will be restored on context switch.
    struct process_context context;
    /// State of the process.
    enum process_state state;
    /// State union depending on the state.
    union {
        /// Valid for `PROCESS_SCHED_ACTIVE` or `PROCESS_SCHED_AVAILABLE`.
        struct process_state_sched sched;
        /// Valid for `PROCESS_WAIT_CHILD`.
        struct process_state_wait_child wait_child;
        /// Valid for `PROCESS_WAIT_TIME`.
        struct process_state_wait_time wait_time;
        /// Valid for `PROCESS_ZOMBIE`.
        struct process_state_zombie zombie;
    };
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
///
/// The process must be in a PROCESS_SCHED_* state.
void process_sched_ring_insert(struct process *process);
/// Internal function used to remove a process from its scheduler ring.
/// Returns the next process after the removed one, might be null if
/// the ring is now empty. The next process returned is guaranteed to
/// be in a `PROCESS_SCHED_*` state.
///
/// The process must be in a `PROCESS_SCHED_*` state, after execution 
/// of this function the process state must be modified to a 
/// non-scheduler state.
///
/// If the process is not in a ring, nothing is done.
struct process *process_sched_ring_remove(struct process *process);
/// Internal function used to find a non-null schedule ring up to
/// and excluding the given priority. This should not return null
/// since at least ring 0 should contain idle process.
struct process *process_sched_ring_find(int max_priority);
/// The active process must no longer be in ACTIVE state when calling 
/// this function.
///
/// If the active process is still in SCHED_AVAILABLE state then the
/// next process of the ring is used (only if next process isn't 
/// forced).
///
/// The next process state is set to ACTIVE, and the previous process
/// state is untouched, so the caller must set it before calling this
/// function.
void process_sched_advance(struct process *next_process);
/// Change the scheduling priority of the given process, this function
/// automatically handles context switch if the next priority is
/// higher than the current process. Previous priority is returned.
///
/// The given process must not be a zombie.
int process_sched_set_priority(struct process *process, int new_priority);
/// Internal function that handle pit interrupts for scheduler.
void process_sched_pit_handler(uint32_t clock);

/// Add the process to the clock queue, the process must be in the
/// `PROCESS_WAIT_TIME` state with corresponding data.
void process_time_queue_add(struct process *process);
/// Remove the process from the clock queue. The process musst be in 
/// the `PROCESS_WAIT_TIME` state with corresponding data.
void process_time_queue_remove(struct process *process);
/// Internal function that handle pit interrupts for clock. It will
/// automatically reschedule processes that reach their target clock.
void process_time_pit_handler(uint32_t clock);

/// Register the process in the overall linked list.
void process_overall_add(struct process *process);
/// Remove the process from the overall linked list.
void process_overall_remove(struct process *process);
/// Get a process from its PID.
struct process *process_from_pid(pid_t pid);

#endif
