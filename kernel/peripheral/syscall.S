.globl syscall_handler
syscall_handler:

    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    leal syscall_handlers, %ebx
    # EBX contains the handlers base, move 4 by 4 in the table.
    movl (%ebx,%eax,4), %ebx
    # Call the resolved address, this function will use.
    call %ebx

    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi

    iret
