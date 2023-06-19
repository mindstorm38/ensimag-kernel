.globl syscall_handler
syscall_handler:
    
    push %edi
    push %esi
    push %edx
    push %ecx
    push %ebx

    lea syscall_handlers, %ecx
    # EBX contains the handlers base, move 4 by 4 in the table.
    mov (%ecx,%eax,4), %ecx
    # Call the resolved address, CDECL call. conv. requires that only
    # EAX, ECX and EDX are not callee-saved.
    call %ecx

    pop %ebx
    pop %ecx
    pop %edx
    pop %esi
    pop %edi

    iret
    # Note: after this, the kernel stack is empty.
