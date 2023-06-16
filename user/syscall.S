// Assembly definition of syscall jumpers.
// We don't need to save registers in these function because the
// interruption handler will do it.

.globl syscall0
syscall0:
    movl 4(%esp), %eax
    int $49
    ret

.globl syscall1
syscall1:
    movl 4(%esp), %eax
    movl 8(%esp), %ebx
    int $49
    ret

.globl syscall2
syscall2:
    movl 4(%esp), %eax
    movl 8(%esp), %ebx
    movl 12(%esp), %ecx
    int $49
    ret

.globl syscall3
syscall3:
    movl 4(%esp), %eax
    movl 8(%esp), %ebx
    movl 12(%esp), %ecx
    movl 16(%esp), %edx
    int $49
    ret

.globl syscall4
syscall4:
    movl 4(%esp), %eax
    movl 8(%esp), %ebx
    movl 12(%esp), %ecx
    movl 16(%esp), %edx
    movl 20(%esp), %esi
    int $49
    ret

.globl syscall5
syscall5:
    movl 4(%esp), %eax
    movl 8(%esp), %ebx
    movl 12(%esp), %ecx
    movl 16(%esp), %edx
    movl 20(%esp), %esi
    movl 24(%esp), %edi
    int $49
    ret
