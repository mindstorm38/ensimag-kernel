#include "syscall.h"

#include "interrupt.h"
#include "process.h"

#include "stddef.h"
#include "stdio.h"


/// A structure holding all saved registers by the assembly interrupt
/// handler. This can be freely used by syscalls to load their 
/// required parameters.
struct syscall_params {
    size_t edi; // EDI
    size_t esi; // ESI
    size_t ebp; // EBP
    size_t esp; // ESP
    size_t ebx; // EBX
    size_t edx; // EDX
    size_t ecx; // ECX
    size_t eax; // EAX
};

// Type alias for a syscall function handler.
typedef void (*syscall_handler_t)(struct syscall_params *);


/// Syscall interrupt handler defined in assembly.
void syscall_handler_entry(void);

/// Syscall handler in C.
void syscall_handler(struct syscall_params *params) {
    
    printf("syscall_handler(%p)\n", params);
    
    


}


void syscall_init(void) {

    printf("Initializing syscall...\n");

    idt_interrupt_gate(SYSCALL_INTERRUPT, (uint32_t) syscall_handler_entry, 3);

}



// Define all syscall handlers.
static syscall_handler_t syscall_handlers[SYSCALL_COUNT] = {
    [SC_PROCESS_START] = process_start, // FIXME:
};
