#include "internals.h"

#include "../debug/user_stack_mem.h"
#include "process.h"
#include "segment.h"
#include "memory.h"
#include "pool.h"

#include "string.h"
#include "stdio.h"


#define PROCESS_POOL_CAP 1024

static id_pool_t(PROCESS_POOL_CAP) process_id_pool = { 0 };
static struct process *process_pool[PROCESS_POOL_CAP] = { 0 };


/// Internal function that allocate a process given. Callers of this
/// function ('process_idle' and 'process_start' only) need to 
/// initialize remaining fields (parent/child/sibling/state).
/// 
/// Interrupts must be disabled before calling this function.
struct process *process_alloc(process_entry_t entry, size_t stack_size, int priority, const char *name, void *arg) {

    if (id_pool_empty(process_id_pool))
        return NULL;

    // FIXME: This value need to be adjusted for function called at 
    // exit or in interrupts, it's not easy to get it right, for 
    // example using printf in those function requires 128 more words.
    size_t more_size = sizeof(size_t) * 32;
    size_t prelude_size = sizeof(size_t) * 2;

    // Check overflow while adding prelude size.
    if (__builtin_add_overflow(stack_size, prelude_size + more_size, &stack_size))
        return NULL;

    struct process *process = kalloc(sizeof(struct process));
    if (process == NULL)
        return NULL;
    
    void *kernel_stack = kalloc(KERNEL_STACK_SIZE);
    if (kernel_stack == NULL) {
        kfree(process);
        return NULL;
    }

    void *stack = user_stack_alloc(stack_size);
    if (stack == NULL) {
        kfree(process);
        kfree(kernel_stack);
        return NULL;
    }

    // Initialize user stack...
    process->stack = stack;
    process->stack_size = stack_size;

    void *stack_top = stack + stack_size;
    size_t *stack_ptr = stack_top - prelude_size;
    stack_ptr[1] = (size_t) arg;
    stack_ptr[0] = (size_t) process_implicit_exit;

    // Initialize kernel stack...
    process->kernel_stack = kernel_stack;

    void *kernel_stack_top = process->kernel_stack + KERNEL_STACK_SIZE;
    size_t kernel_stack_prelude = sizeof(uint32_t) * (5 + 5);
    uint32_t *kernel_stack_ptr = kernel_stack_top - kernel_stack_prelude;

    // Used by the startup function, that just call "iret"...
    kernel_stack_ptr[9] = USER_DS;              // SS     (for iret)
    kernel_stack_ptr[8] = (uint32_t) stack_ptr; // ESP    (for iret)
    kernel_stack_ptr[7] = 0x202;                // EFLAGS (for iret)
    kernel_stack_ptr[6] = USER_CS;              // CS     (for iret)
    kernel_stack_ptr[5] = (uint32_t) entry;     // EIP    (for iret)
    // Used by the first context switch...
    kernel_stack_ptr[4] = (uint32_t) process_context_startup; // EIP (for ret)
    kernel_stack_ptr[3] = 0; // EBP
    kernel_stack_ptr[2] = 0; // EDI
    kernel_stack_ptr[1] = 0; // ESI
    kernel_stack_ptr[0] = 0; // EBX

#if PROCESS_DEBUG
    printf("[?] process_alloc(...) kernel_stack_top: %p, stack_ptr: %p\n", kernel_stack_top, stack_ptr);
#endif

    process->kernel_esp = (uint32_t) kernel_stack_ptr;

    // Initialize other fields
    strcpy(process->name, name);

    // Priority and scheduler ring.
    process->priority = priority;
    process->state = PROCESS_SCHED;
    process_sched_ring_insert(process);
    
    // Allocate the PID.
    process->pid = id_pool_alloc(process_id_pool);
    process_pool[process->pid] = process;

    return process;

}

/// Internal function to free the given process. Caller must ensure
/// that this process is present in the overall list. The process
/// should not be the active one.
/// 
/// This is typically used when the process has been a zombie and is
/// waited for by its parent, or if its parent dies.
void process_free(struct process *process) {

    if (process->state != PROCESS_ZOMBIE) {
        panic("process_free(...): process->state not ZOMBIE");
    }

    // Remove it from its parent/sibling relationship.
    struct process *parent = process->parent;
    if (parent != NULL) {

        struct process **child_ptr = &parent->child;
        while (*child_ptr != NULL) {
            if (*child_ptr == process) {
                // We found the child to remove, set the parent ptr to
                // its sibling (which may be null if no sibling).
                *child_ptr = process->sibling;
                break;
            }
            child_ptr = &(*child_ptr)->sibling;
        }

    }

    // We need to null-ptr the child's parent, to avoid storing an
    // invalid pointer.
    struct process *child = process->child;
    while (child != NULL) {
        child->parent = NULL;
        child = child->sibling;
    }

    // Free resources.
    kfree(process);
    kfree(process->kernel_stack);
    user_stack_free(process->stack, process->stack_size);

    process_pool[process->pid] = NULL;
    id_pool_free(process_id_pool, process->pid);

}

struct process *process_from_pid(pid_t pid) {
    if (pid < 0 || pid >= PROCESS_POOL_CAP) {
        return NULL;
    } else {
        return process_pool[pid];
    }
}
