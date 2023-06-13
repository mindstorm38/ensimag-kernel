#include "process_internals.h"
#include "memory.h"
#include "stdio.h"

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

/// This function find the next process that should have higher 
/// priority in the given list. Returning null if no process in
/// the queue.
static struct process *process_queue_pop_next(struct process_queue *queue) {

    struct process **wait_process_ptr = &queue->waiting_process;
    struct process **wake_process_ptr = NULL;

    while (*wait_process_ptr != NULL) {

        // If all processes in the list have the same priority, then
        // the last one will be the awaken one.
        if (wake_process_ptr == NULL || (*wake_process_ptr)->priority <= (*wait_process_ptr)->priority)
            wake_process_ptr = wait_process_ptr;

        wait_process_ptr = &(*wait_process_ptr)->wait_queue.next;

    }

    struct process *wake_process = NULL;

    if (wake_process_ptr != NULL) {
        // Remove this process from the waiting ones.
        wake_process = *wake_process_ptr;
        *wake_process_ptr = wake_process->wait_queue.next;
    }

    return wake_process;

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

        process_sched_ring_insert(waiting_process);

        waiting_process = waiting_process->wait_queue.next;

    }

    queue->waiting_process = NULL;

    // The highest priority process has greater priority than running
    // process? Schedule it.
    if (wake_process != NULL && wake_process->priority > process_active->priority) {
        process_active->state = PROCESS_SCHED_AVAILABLE;
        process_sched_advance(wake_process);
    }

}

qid_t process_queue_create(size_t capacity) {

#if QUEUE_DEBUG
    printf("[%s] process_queue_create(%d)\n", process_active->name, capacity);
#endif

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

#if QUEUE_DEBUG
    printf("[%s] process_queue_delete(%d)\n", process_active->name, qid);
#endif

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

#if QUEUE_DEBUG
    printf("[%s] process_queue_send(%d, %d)\n", process_active->name, qid, message);
#endif

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;

    if (queue->length == queue->capacity) {
        
        // Queue is full, just block this process until the message
        // has been added.
        struct process *next_process = process_sched_ring_remove(process_active);

        process_active->state = PROCESS_WAIT_QUEUE;
        process_active->wait_queue.next = queue->waiting_process;
        process_active->wait_queue.message = message;
        queue->waiting_process = process_active;

        // Schedule a new process.
        process_sched_advance(next_process);

        // When resuming, we know that the receiving process has
        // written our message.
        if (process_active->sched.wait_queue_reset)
            return -1;

    } else {
        
        queue->messages[queue->write_index] = message;

        if (++queue->write_index == queue->capacity)
            queue->write_index = 0;

        if (queue->length++ == 0) {
            
            // If queue capacity was 0 and a process is waiting (must
            // be for reading) then we re-schedule it.
            struct process *next_process = process_queue_pop_next(queue);
            if (next_process == NULL)
                return 0;

            next_process->state = PROCESS_SCHED_AVAILABLE;
            next_process->sched.wait_queue_reset = false;

            process_sched_ring_insert(next_process);

            if (next_process->priority > process_active->priority) {
                process_active->state = PROCESS_SCHED_AVAILABLE;
                process_sched_advance(next_process);
            }

        }

    }

    return 0;

}

int process_queue_receive(qid_t qid, int *message) {

#if QUEUE_DEBUG
    printf("[%s] process_queue_receive(%d, %p)\n", process_active->name, qid, message);
#endif

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;
    
    if (queue->length == 0) {

        // The queue is empty so we wait for a space.
        struct process *next_process = process_sched_ring_remove(process_active);

        process_active->state = PROCESS_WAIT_QUEUE;
        process_active->wait_queue.next = queue->waiting_process;
        queue->waiting_process = process_active;

        // Schedule a new process.
        process_sched_advance(next_process);

        if (process_active->sched.wait_queue_reset)
            return -1;

    }

    if (message != NULL) {
        *message = queue->messages[queue->read_index];
    }

    if (++queue->read_index == queue->capacity)
        queue->read_index = 0;
    
    if (queue->length-- == queue->capacity) {

        // Queue was full, we get highest priority process and 
        // instantly write its message to the queue.
        struct process *next_process = process_queue_pop_next(queue);
        if (next_process == NULL)
            return 0;

        queue->messages[queue->write_index] = next_process->wait_queue.message;

        if (++queue->write_index == queue->capacity)
            queue->write_index = 0;

        queue->length++;
        
        next_process->state = PROCESS_SCHED_AVAILABLE;
        next_process->sched.wait_queue_reset = false;
            
        process_sched_ring_insert(next_process);

        if (next_process->priority > process_active->priority) {
            process_active->state = PROCESS_SCHED_AVAILABLE;
            process_sched_advance(next_process);
        }

    }

    return 0;

}

int process_queue_count(qid_t qid, int *count) {

#if QUEUE_DEBUG
    printf("[%s] process_queue_count(%d, %p)\n", process_active->name, qid, count);
#endif

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;
    
    if (count == NULL)
        return 0;

    int waiting_count = 0;
    struct process *waiting_process = queue->waiting_process;
    while (waiting_process != NULL) {
        waiting_count++;
        waiting_process = waiting_process->wait_queue.next;
    }

#if QUEUE_DEBUG
    printf("[%s] process_queue_count(...): waiting_process = %d, len = %d\n", process_active->name, waiting_count, queue->length);
#endif

    if (queue->length == 0) {
        *count = -waiting_count;
    } else if (queue->length == queue->capacity) {
        *count = queue->length + waiting_count;
    } else {

        // // Check that no process is waiting if not expected.
        // if (waiting_count != 0) {
        //     panic("process_queue_count(...): processes are waiting but space is available");
        // }

        *count = 0;

    }

    return 0;

}

int process_queue_reset(qid_t qid) {

#if QUEUE_DEBUG
    printf("[%s] process_queue_count(%d)\n", process_active->name, qid);
#endif

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;

    process_queue_resume_reset(queue);
    return 0;

}
