#ifndef __PROCESS_INTERNALS_H__
#define __PROCESS_INTERNALS_H__

#include "stdbool.h"
#include "stdint.h"

#include "process.h"
#include "arch.h"


// Note that enabling debug may fail some kill(...) or exit(...) test
// because of insufficient stack size, but that's okay.
#define PROCESS_DEBUG 0
#define QUEUE_DEBUG 0

#define KERNEL_STACK_SIZE 512


// TODO: May be a good idea to have a more generic wait support, with
// generic wait but with different peripherals behind. Like time and
// console read for exemple. But it's a bit too complicated for now.


struct process;
struct process_queue;

/// States that a process can take, used for scheduling.
enum process_state {
    /// The process is the currently executed one, it must be in the
    /// scheduler rings of its priority.
    PROCESS_SCHED_ACTIVE,
    /// The process is in the scheduler ring and is waiting to be
    /// executed.
    PROCESS_SCHED_AVAILABLE,
    /// The process is waiting for termination of one of its children.
    PROCESS_WAIT_CHILD,
    /// The process is waiting for reaching a target clock count.
    PROCESS_WAIT_TIME,
    /// The process is waiting to read/write from/to a process queue.
    PROCESS_WAIT_QUEUE,
    /// The process is waiting for some IO to wake it up.
    PROCESS_WAIT_CONS_READ,
    /// The process is dead and is waiting termination by its parent,
    /// if the parent is itself a zombie, the process is just freed.
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
    /// The process is resumed from `PROCESS_WAIT_QUEUE` because it
    /// was waiting to receive a message, the message is given here.
    int wait_queue_message;
    /// Used when resuming from the `PROCESS_WAIT_QUEUE`, it indicates
    /// if the process was resumed by a reset.
    bool wait_queue_reset;
};

/// For process that are in `PROCESS_WAIT_CHILD`.
struct process_state_wait_child {
    /// The pid that is waited for (negative for any child).
    pid_t child_pid;
};

struct process_state_wait_time {
    /// Next process in the wait time linked list.
    struct process *next;
    /// Target clock time at which the process should be rescheduled.
    uint32_t target_clock;
};

struct process_state_wait_queue {
    /// Previous process in the wait queue linked list.
    struct process *prev;
    /// Next process in the wait queue linked list.
    struct process *next;
    /// Pointer to the queue currently used by the process.
    struct process_queue *queue;
    /// The message waiting to be written.
    int message;
};

struct process_state_wait_cons_read {
    /// Next process in the wait cons read linked list.
    struct process *next;
};

/// Zombie-specific state for process that are in `PROCESS_ZOMBIE`.
struct process_state_zombie {
    /// Exit code of the process.
    int exit_code;
};

// /// Registers context of a process.
// struct process_context {
//     uint32_t ebx;
//     uint32_t esp;
//     uint32_t ebp;
//     uint32_t esi;
//     uint32_t edi;
// };

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
    /// State of the process.
    enum process_state state;
    /// State "tagged union" depending on the state.
    union {
        /// Valid for `PROCESS_SCHED_ACTIVE` or `PROCESS_SCHED_AVAILABLE`.
        struct process_state_sched sched;
        /// Valid for `PROCESS_WAIT_CHILD`.
        struct process_state_wait_child wait_child;
        /// Valid for `PROCESS_WAIT_TIME`.
        struct process_state_wait_time wait_time;
        /// Valid for `PROCESS_WAIT_QUEUE`.
        struct process_state_wait_queue wait_queue;
        /// Valid for `PROCESS_WAIT_CONS_READ`.
        struct process_state_wait_cons_read wait_cons_read;
        /// Valid for `PROCESS_ZOMBIE`.
        struct process_state_zombie zombie;
    };
    /// Kernel stack, it is really important and we use it to execute
    /// our interrupt handler so we can resume the execution of the
    /// kernel code when the process is resumed. 
    /// 
    /// It also contains the saved registers of the user code, that 
    /// are saved by interrupt handlers for syscalls or other 
    /// interrupts.
    char *kernel_stack;
    /// The kernel's current ESP register value, we only keep this 
    /// value because the kernel code's context is saved in the 
    /// kernel's stack.
    uint32_t kernel_esp;
    /// The process' stack.
    char *stack;
    /// The stack size.
    size_t stack_size;
    /// Process id, its index in the internal process queue.
    pid_t pid;
    /// Name of the process.
    char name[PROCESS_NAME_MAX_SIZE];
    /// Scheduling priority, from 0 to PROCESS_MAX_PRIORITY excluded.
    int priority;
};

struct process_queue {
    /// When the queue is free to be used, it is registered here.
    struct process_queue *next_free_queue;
    /// Queue ID.
    qid_t qid;
    /// Message list allocation. This is null when the queue is not
    /// present (sentinel).
    int *messages;
    /// Capacity of the queue.
    size_t capacity;
    /// Internal message count.
    size_t length;
    /// Read index of the queue.
    size_t read_index;
    /// Write index of the queue.
    size_t write_index;
    /// Head of the waiting processes. When length is equal to 
    /// capacity then theses processes are waiting for writing, if
    /// length is equal to zero then the processes are waiting for
    /// reading.
    struct process *wait_process;
};


/// The pointer to the currently active process being executed at user
/// level.
extern struct process *process_active;


/// Register the process in the overall linked list. This also set
/// the PID of the given process.
void process_overall_add(struct process *process);
/// Remove the process from the overall linked list.
void process_overall_remove(struct process *process);
/// Get a process from its PID.
struct process *process_from_pid(pid_t pid);

/// Context switch between a previous process and a next one.
/// This function must be called from kernel privilege level and will
/// resume the next process in the user privilege level.
void process_context_switch(struct process *prev, struct process *next);
/// Startup function that is called once when first starting a 
/// user privilege level process.
void process_context_startup();

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
/// The next process state is set to ACTIVE.
void process_sched_advance(struct process *next_process);
/// Change the scheduling priority of the given process, this function
/// automatically handles context switch if the next priority is
/// higher than the current process.
///
/// The given process must be in `PROCESS_SCHED_*` state.
void process_sched_set_priority(struct process *process, int new_priority);
/// Internal function that handle pit interrupts for scheduler.
void process_sched_pit_handler(uint32_t clock);

/// Add the process to the clock queue, the process must be in the
/// `PROCESS_WAIT_TIME` state with corresponding data.
void process_time_queue_add(struct process *process);
/// Remove the process from the clock queue. The process must be in 
/// the `PROCESS_WAIT_TIME` state with corresponding data.
void process_time_queue_remove(struct process *process);
/// Internal function that handle pit interrupts for clock. It will
/// automatically reschedule processes that reach their target clock.
void process_time_pit_handler(uint32_t clock);

/// Change the priority of a process that is currently waiting for a
/// queue. The process must be in `PROCESS_WAIT_QUEUE` state.
void process_queue_set_priority(struct process *process, int new_priority);
/// Kill a process that is waiting for queue. The process must be in
/// `PROCESS_WAIT_QUEUE` state.
void process_queue_kill_process(struct process *process);

/// Kill a process that is waiting for a console read. The process 
/// must be in `PROCESS_WAIT_CONS_READ` state.
void process_cons_read_kill_process(struct process *process);

/// Internal function to debug print a process.
void process_debug(struct process *process);

#endif
