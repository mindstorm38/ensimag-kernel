.globl process_context_switch_kernel
process_context_switch_kernel:

    # We know that the caller has pushed the returned address on the
    # stack here, so we push saved registers after the return address.

    # Move the "prev_esp" pointer to EAX and "next_esp" to ECX.
    movl 4(%esp), %eax
    movl 8(%esp), %ecx

    # We are in the process' kernel stack, we save the callee-saved
    # registers and then ESP. Note: EAX, ECX, EDX are caller-saved.
    pushl %ebx
    pushl %esi
    pushl %edi
    pushl %ebp

    # The final previous process' kernel stack layout is:
    # +-----+
    # | EIP |
    # | EBX |
    # | ESI |
    # | EDI |
    # | EBP |
    # +-----+

    # Move ESP to the "prev_esp" pointer stored in EAX.
    movl %esp, (%eax)

    # Then we restore the next ESP to the register.
    movl %ecx, %esp

    # From now on, we are in the next process' kernel stack.
    # We now resume the callee-saved registers like above:
    popl %ebp
    popl %edi
    popl %esi
    popl %ebx

    # The only thing remaining in stack is EIP, so we return.
    ret


.globl process_context_startup
process_context_startup:

    # We arrive in this function at process' startup, this function is
    # "called" when the first context switch called on a new process,
    # we arrive here and the only remaining stack element is the 
    # user code process' entry.
    
    # The process' kernel stack already contains all informations for
    # using iret apart from the EFLAGS register that we just init
    # below by pushing our current EFLAGS.
    pushfl
    popl %eax
    # # Set IF (Interrupt Flag), interrupts are not masked in used mode.
    # orl $0x200, %eax
    # Move our new EFLAGS to the EFLAGS slot.
    movl %eax, 8(%esp)

    iret


.globl process_implicit_exit
process_implicit_exit:
    pushl %eax
    call process_internal_exit
