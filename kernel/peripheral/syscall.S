.globl syscall_handler_entry
syscall_handler_entry:
    pushal
    pushl %esp
    call syscall_handler
    addl $4, %esp
    popal
    iret
