.globl syscall_handler
syscall_handler:

    push %edi
    push %esi
    push %edx
    push %ecx
    push %ebx

    lea syscall_handlers, %ebx
    # EBX contains the handlers base, move 4 by 4 in the table.
    mov (%ebx,%eax,4), %ebx
    # Call the resolved address, CDECL call. conv. requires that only
    # EAX, ECX and EDX are not callee-saved.
    call %ebx

    pop %ebx
    pop %ecx
    pop %edx
    pop %esi
    pop %edi

    iret
    # Note: after this, the kernel stack is empty.
