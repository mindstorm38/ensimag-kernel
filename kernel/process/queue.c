#include "internals.h"

#include "process.h"
#include "memory.h"
#include "pool.h"

#include "stdio.h"


#define QUEUE_POOL_CAP 256

static id_pool_t(QUEUE_POOL_CAP) queue_id_pool = { 0 };
static struct process_queue *queue_pool[QUEUE_POOL_CAP] = { 0 };


/// Get a queue pointer from its queue ID, while checking the validity
/// of the QID.
static struct process_queue *process_queue_from_qid(qid_t qid) {
    if (qid < 0 || qid >= QUEUE_POOL_CAP) {
        return NULL;
    } else {
        return queue_pool[qid];
    }
}

/// Add the given process to the queue's waiting list. The process
/// must be in WAIT_QUEUE state.
static void process_queue_add_process(struct process_queue *queue, struct process *process) {
    process->wait_queue.prev = NULL;
    process->wait_queue.next = queue->wait_process;
    if (queue->wait_process != NULL)
        queue->wait_process->wait_queue.prev = process;
    queue->wait_process = process;
}

/// Remove the given process from the queue's waiting list. The 
/// process must be in WAIT_QUEUE state.
static void process_queue_remove_process(struct process_queue *queue, struct process *process) {
    // If it's the head of the queue.
    if (process->wait_queue.prev == NULL) {
        queue->wait_process = process->wait_queue.next;
        if (queue->wait_process != NULL)
            queue->wait_process->wait_queue.prev = NULL;
    } else {
        struct process *prev_process = process->wait_queue.prev;
        struct process *next_process = process->wait_queue.next;
        prev_process->wait_queue.next = next_process;
        if (next_process != NULL)
            next_process->wait_queue.prev = prev_process;
    }
}

/// Put the active process in wait state for the given queue. 
/// The message can be given if relevant (writing wait).
/// The function returns true if resuming is due to a reset.
static bool process_queue_wait(struct process_queue *queue, int message) {

    // Queue is full, just block this process until the message
    // has been added.
    struct process *next_process = process_sched_ring_remove(process_active);
    process_active->state = PROCESS_WAIT_QUEUE;
    process_active->wait_queue.message = message;
    process_active->wait_queue.queue = queue;
    process_queue_add_process(queue, process_active);

    // Schedule a new process.
    process_sched_advance(next_process);

    // When resuming, we know that the receiving process has
    // written our message.
    return process_active->sched.wait_queue_reset;

}

/// This function find the next process that should have higher 
/// priority in the given list. Returning null if no process in
/// the queue.
static struct process *process_queue_pop_next(struct process_queue *queue) {

    struct process *wait_process = queue->wait_process;
    struct process *candidate_process = NULL;

    while (wait_process != NULL) {

        // If all processes in the list have the same priority, then
        // the last one will be the awaken one.
        if (candidate_process == NULL || candidate_process->priority <= wait_process->priority)
            candidate_process = wait_process;

        wait_process = wait_process->wait_queue.next;

    }

    if (candidate_process != NULL) {
        process_queue_remove_process(queue, candidate_process);
    }

    return candidate_process;

}

/// Resume all waiting processes and set the reset flag to true so
/// they will return -1 on return. The given process must be in WAIT_QUEUE
static void process_queue_resume_reset(struct process **wait_process_ptr) {

    struct process *wait_process = *wait_process_ptr;
    struct process *wake_process = NULL;

    while (wait_process != NULL) {

        if (wake_process == NULL || wake_process->priority < wait_process->priority)
            wake_process = wait_process;
        
        // Save the next process here because state is changed.
        struct process *next_process = wait_process->wait_queue.next;

        wait_process->state = PROCESS_SCHED;
        wait_process->sched.wait_queue_reset = true;
        process_sched_ring_insert(wait_process);

        wait_process = next_process;

    }
    
    *wait_process_ptr = NULL;

    // The highest priority process has greater priority than running
    // process? Schedule it.
    if (wake_process != NULL && wake_process->priority > process_active->priority) {
        process_sched_advance(wake_process);
    }

}

/// Send a message to a queue (without checking if space is available).
/// It returns the previous length.
static size_t process_queue_raw_write(struct process_queue *queue, int message) {
    queue->messages[queue->write_index] = message;
    if (++queue->write_index == queue->capacity)
        queue->write_index = 0;
    return queue->length++;
}

/// Receive a message from a queue (without checking if space is available).
/// It returns the previous length.
static size_t process_queue_raw_read(struct process_queue *queue, int *message) {
    if (message != NULL)
        *message = queue->messages[queue->read_index];
    if (++queue->read_index == queue->capacity)
        queue->read_index = 0;
    return queue->length--;
}

qid_t process_queue_create(size_t capacity) {

#if QUEUE_DEBUG
    printf("[%s] process_queue_create(%d)\n", process_active->name, capacity);
#endif

    if (capacity == 0 || id_pool_empty(queue_id_pool))
        return -1;

    size_t messages_alloc;
    if (__builtin_mul_overflow(sizeof(int), capacity, &messages_alloc))
        return -1;

    int *messages = kalloc(messages_alloc);
    if (messages == NULL)
        return -1;

    struct process_queue *queue = kalloc(sizeof(struct process_queue));
    if (queue == NULL) {
        kfree(messages);
        return -1;
    }

    queue->messages = messages;
    queue->capacity = capacity;
    queue->length = 0;
    queue->read_index = 0;
    queue->write_index = 0;
    queue->wait_process = NULL;

    queue->qid = id_pool_alloc(queue_id_pool);
    queue_pool[queue->qid] = queue;

    return queue->qid;

}

int process_queue_delete(qid_t qid) {

#if QUEUE_DEBUG
    printf("[%s] process_queue_delete(%d)\n", process_active->name, qid);
#endif

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;

    struct process *process = queue->wait_process;

    queue_pool[queue->qid] = NULL;
    id_pool_free(queue_id_pool, queue->qid);
    
    process_queue_resume_reset(&process);

    kfree(queue->messages);
    kfree(queue);

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

#if QUEUE_DEBUG
        printf("[%s] process_queue_send(...): length == capacity (%d)\n", process_active->name, queue->length);
#endif

        if (process_queue_wait(queue, message))
            return -1;

    } else {

        if (queue->length == 0) {

            // If queue capacity was 0 and a process is waiting (must
            // be for reading) then we re-schedule it.
            struct process *next_process = process_queue_pop_next(queue);
            if (next_process != NULL) {

                next_process->state = PROCESS_SCHED;
                next_process->sched.wait_queue_reset = false;
                next_process->sched.wait_queue_message = message;

                process_sched_ring_insert(next_process);

                if (next_process->priority > process_active->priority) {
                    process_sched_advance(next_process);
                }

                return 0;

            }

        }

        process_queue_raw_write(queue, message);

    }

    return 0;

}

int process_queue_receive(qid_t qid, int *message) {

#if QUEUE_DEBUG
    printf("[%s] process_queue_receive(%d, %p)\n", process_active->name, qid, message);
#endif

    if (message != NULL && !process_check_user_ptr(message))
        return -1;

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;
    
    if (queue->length == 0) {

#if QUEUE_DEBUG
        printf("[%s] process_queue_receive(...): length == 0\n", process_active->name);
#endif

        if (process_queue_wait(queue, 0))
            return -1;
        
        // Directly receive the message.
        if (message != NULL) {
            *message = process_active->sched.wait_queue_message;
        }

    } else {

        if (process_queue_raw_read(queue, message) == queue->capacity) {

            // Queue was full, we get highest priority process and 
            // instantly write its message to the queue.
            struct process *next_process = process_queue_pop_next(queue);
            if (next_process == NULL)
                return 0;

#if QUEUE_DEBUG
            printf("[%s] process_queue_receive(...): queue was full, receiving %d from %s\n", process_active->name, next_process->wait_queue.message, next_process->name);
#endif

            process_queue_raw_write(queue, next_process->wait_queue.message);

            next_process->state = PROCESS_SCHED;
            next_process->sched.wait_queue_reset = false;
            next_process->sched.wait_queue_message = -1;

            process_sched_ring_insert(next_process);

            if (next_process->priority > process_active->priority) {
                process_sched_advance(next_process);
            }

        }

    }

    return 0;

}

int process_queue_count(qid_t qid, int *count) {

#if QUEUE_DEBUG
    printf("[%s] process_queue_count(%d, %p)\n", process_active->name, qid, count);
#endif

    if (count != NULL && !process_check_user_ptr(count))
        return -1;

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;
    
    if (count == NULL)
        return 0;

    int waiting_count = 0;
    struct process *waiting_process = queue->wait_process;
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
    printf("[%s] process_queue_reset(%d)\n", process_active->name, qid);
#endif

    struct process_queue *queue = process_queue_from_qid(qid);
    if (queue == NULL)
        return -1;

    queue->length = 0;
    queue->read_index = 0;
    queue->write_index = 0;

    process_queue_resume_reset(&queue->wait_process);

    return 0;

}

void process_queue_set_priority(struct process *process, int new_priority) {

    process->priority = new_priority;

    // Simply remove and add again the process.
    process_queue_remove_process(process->wait_queue.queue, process);
    process_queue_add_process(process->wait_queue.queue, process);

}

void process_queue_kill_process(struct process *process) {
    process_queue_remove_process(process->wait_queue.queue, process);
}
