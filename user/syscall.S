// Assembly definition of syscall jumpers.
// We don't need to save registers in these function because the
// interruption handler will do it.

.globl syscall0
syscall0:
    mov 4(%esp), %eax
    int $49
    ret

.globl syscall1
syscall1:
    push %ebx  # Callee-saved
    mov 8(%esp), %eax
    mov 12(%esp), %ebx
    int $49
    pop %ebx
    ret

.globl syscall2
syscall2:
    push %ebx  # Callee-saved
    mov 8(%esp), %eax
    mov 12(%esp), %ebx
    mov 16(%esp), %ecx
    int $49
    pop %ebx
    ret

.globl syscall3
syscall3:
    push %ebx  # Callee-saved
    mov 8(%esp), %eax
    mov 12(%esp), %ebx
    mov 16(%esp), %ecx
    mov 20(%esp), %edx
    int $49
    pop %ebx
    ret

.globl syscall4
syscall4:
    push %ebx  # Callee-saved
    push %esi
    mov 12(%esp), %eax
    mov 16(%esp), %ebx
    mov 20(%esp), %ecx
    mov 24(%esp), %edx
    mov 28(%esp), %esi
    int $49
    pop %esi
    pop %ebx
    ret

.globl syscall5
syscall5:
    push %ebx  # Callee-saved
    push %esi
    push %edi
    mov 16(%esp), %eax
    mov 20(%esp), %ebx
    mov 24(%esp), %ecx
    mov 28(%esp), %edx
    mov 32(%esp), %esi
    mov 36(%esp), %edi
    int $49
    pop %edi
    pop %esi
    pop %ebx
    ret
