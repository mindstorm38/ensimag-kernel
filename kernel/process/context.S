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
    # | RA  |
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

    # The only thing remaining in stack is RA, so we return.
    ret


.globl process_context_startup
process_context_startup:

    # We arrive in this function at process' startup, this function is
    # "called" when the first context switch called on a new process,
    # we arrive here and the only remaining stack element is the 
    # user code process' entry.

    # Stack layout is:
    # +--------+
    # | SS     |
    # | ESP    |
    # | EFLAGS |
    # | CS     |
    # | EIP    |
    # +--------+

    # Set the DS segment to the same as SS.
    movl 16(%esp), %eax
    movw %ax, %ds

    # Don't leak kernel address to user code.
    movl $0, %eax

    iret


.globl process_implicit_exit
.section .kernel_user
process_implicit_exit:
    # pushl %eax
    
    # Here, %eax contains the return code.
    # Implicit exit just fowards to exit syscall 'SC_PROCESS_EXIT' (1)

    movl %eax, %ebx
    movl $1, %eax
    int $49

    # Should not terminate here.
0:
    jmp 0b
    

    # call process_internal_exit
