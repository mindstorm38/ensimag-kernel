#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "stdint.h"
#include "stddef.h"

#define PROCESS_NAME_MAX_SIZE 128
#define PROCESS_STACK_SIZE 512
#define PROCESS_MAX_COUNT 2

#define PROCESS_MAX_PRIORITY 256


typedef int pid_t;
typedef int qid_t;

/// Type alias for process entry point.
typedef int (*process_entry_t)(void *);

/// Startup function that starts the first process. It should be 
/// called only once at kernel startup, this process can then starts
/// other threads.
void process_idle(process_entry_t entry, size_t stack_size, void *arg);
/// Start a process.
pid_t process_start(process_entry_t entry, size_t stack_size, int priority, const char *name, void *arg);
/// Exit from the current process.
void process_exit(int code) __attribute__((noreturn));

/// Get PID of the current process.
pid_t process_pid(void);
/// Get name of the current process.
char *process_name(void);

/// Return the priority of the given pid.
int process_priority(pid_t pid);
/// Set priority and return previous one.
int process_set_priority(pid_t pid, int priority);

/// Wait for termination of one of the child processes.
pid_t process_wait(pid_t pid, int *exit_code);
/// Kill the given process by pid.
int process_kill(pid_t pid);

/// Pause the process for given number of clock cycles.
void process_wait_clock(uint32_t clock);

/// Create a message queue with the given capacity of messages.
qid_t process_queue_create(size_t capacity);
/// Delete a queue from its ID.
int process_queue_delete(qid_t qid);
/// Send a message to a queue of given ID.
int process_queue_send(qid_t qid, int message);
/// Receive a message from a queue of given ID.
int process_queue_receive(qid_t qid, int *message);
/// Count waiting processes and messages on a queue of given ID.
int process_queue_count(qid_t qid, int *count);
/// Remove all messages from a queue of given ID.
int process_queue_reset(qid_t qid);

#endif
