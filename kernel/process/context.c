#include "internals.h"
#include "segment.h"
#include "arch.h"

#include "stdio.h"


/// Dummy stack used when previous process is null, to give a fake
/// previous ESP that points to this array. The register will be
/// stored in it be never used.
static uint32_t dummy_stack;

/// Internal function that context switch the previous process' kernel
/// code to the next one. Everything is stored in the process' kernel
/// stack so we only give previous and next ESP.
void process_context_switch_kernel(uint32_t *prev_esp, uint32_t next_esp);


void process_context_switch(struct process *prev, struct process *next) {

    // This function is executing in kernel privilege level and we 
    // want to save the context of the currently executing kernel
    // registers and stack.

    tss.ss0 = KERNEL_DS;
    tss.esp0 = (uint32_t) next->kernel_stack + KERNEL_STACK_SIZE;

    uint32_t *prev_esp = prev == NULL ? &dummy_stack : &prev->kernel_esp;
    process_context_switch_kernel(prev_esp, next->kernel_esp);

}
