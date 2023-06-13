#include "process_internals.h"
#include "memory.h"
#include <stddef.h>

#define QUEUES_CAPACITY 256


static struct process_queue queues[QUEUES_CAPACITY] = {0};
static size_t queues_count = 0;

static struct process_queue *free_queue = NULL;


/// Get a queue pointer from its queue ID, while checking the validity
/// of the QID.
static struct process_queue *process_queue_from_qid(qid_t qid) {

    if (qid < 0 || qid >= QUEUES_CAPACITY)
        return NULL;

    struct process_queue *queue = &queues[qid];
    if (queue->messages == NULL)
        return NULL;
    
    return queue;

}

/// Set the active process to wait. This function will resume when
/// the process will be ready to read.
static bool process_queue_wait(struct process_queue *queue) {

    struct process *next_process = process_sched_ring_remove(process_active);

    process_active->state = PROCESS_WAIT_QUEUE;
    process_active->wait_queue.next = queue->waiting_process;
    queue->waiting_process = process_active;

    // Schedule a new process.
    process_sched_advance(next_process);
    
    // Return if the resuming is due to reset or delete.
    return process_active->sched.wait_queue_reset;

}

/// Resume a waiting process 
static void process_queue_resume(struct process_queue *queue) {
    
    struct process **waiting_process = &queue->waiting_process;
    struct process **wake_process = NULL;

    while (*waiting_process != NULL) {
        
        if (wake_process == NULL || (*wake_process)->priority < (*waiting_process)->priority)
            wake_process = waiting_process;

        waiting_process = &(*waiting_process)->wait_queue.next;

    }

    if (wake_process != NULL) {

        // Remove this process from the waiting ones.
        struct process *higher_wake_process = *wake_process;
        *wake_process = higher_wake_process->wait_queue.next;

        higher_wake_process->state = PROCESS_SCHED_AVAILABLE;
        higher_wake_process->sched.wait_queue_reset = false;

        // Force advance if we wake a higher priority process.
        if (higher_wake_process->priority > process_active->priority)
            process_sched_advance(higher_wake_process);

    }

}

/// Resume all waiting processes and sset the reset flag to true so
/// they will return -1 on return.
static void process_queue_resume_reset(struct process_queue *queue) {

    struct process *waiting_process = queue->waiting_process;
    struct process *wake_process = NULL;

    while (waiting_process != NULL) {

        if (wake_process == NULL || wake_process->priority < waiting_process->priority)
            wake_process = waiting_process;
        
        waiting_process->state = PROCESS_SCHED_AVAILABLE;
        waiting_process->sched.wait_queue_reset = true;

        waiting_process = waiting_process->wait_queue.next;

    }

    queue->waiting_process = NULL;

    // The highest priority process has greater priority than running
    // process? Schedule it.
    if (wake_process != NULL && wake_process->priority > process_active->priority)
        process_sched_advance(wake_process);

}

qid_t process_queue_create(size_t capacity) {

    if (capacity == 0 || queues_count >= QUEUES_CAPACITY)
        return -1;

    struct process_queue *queue;

    int *messages = kalloc(sizeof(int) * capacity);
    if (messages == NULL)
        return -1;

    if (free_queue != NULL) {
        queue = free_queue;
        free_queue = queue->next_free_queue;
    } else {
        qid_t next_id = queues_count++;
        queue = &queues[next_id];
        queue->qid = next_id;
    }

    queue->next_free_queue = NULL;
    queue->capacity = capacity;
    queue->length = 0;
    queue->messages = messages;
    queue->read_index = 0;
    queue->write_index = 0;
    queue->waiting_process = NULL;

    return queue->qid;

}

int process_queue_delete(qid_t qid) {

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;
    
    kfree(queue->messages);
    queue->messages = NULL;

    process_queue_resume_reset(queue);

    queue->next_free_queue = free_queue;
    free_queue = queue;

    return 0;

}

int process_queue_send(qid_t qid, int message) {

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;
    
    // Spin block while no message can be sent.
    while (queue->length == queue->capacity) {
        if (process_queue_wait(queue))
            return -1;
    }

    queue->messages[queue->write_index++] = message;

    // Wrap capacity around.
    if (queue->write_index == queue->capacity)
        queue->write_index = 0;
    
    // If queue capacity was 0 and processes are waiting, wake one.
    if (queue->length++ == 0) {
        process_queue_resume(queue);
    }

    return 0;

}

int process_queue_receive(qid_t qid, int *message) {

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;
    
    // Spin block while no message can be received.
    while (queue->length == 0) {
        if (process_queue_wait(queue))
            return -1;
    }

    *message = queue->messages[queue->read_index++];

    if (queue->read_index == queue->capacity)
        queue->read_index = 0;
    
    // If queue length was cap. and processes are waiting, wake one.
    if (queue->length-- == queue->capacity) {
        process_queue_resume(queue);
    }

    return 0;

}

int process_queue_reset(qid_t qid) {

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;

    process_queue_resume_reset(queue);
    return 0;

}
