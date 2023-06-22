/// Shared syscall numbers and interrupt number.

#ifndef __SYSCALL_SHARED_H__
#define __SYSCALL_SHARED_H__

// Our syscall calling convention uses the following registers:
// - EAX: syscall number and return value
// - EBX: first param
// - ECX: second param
// - EDX: third param
// - ESI: fourth param
// - EDI: fifth param

// Interrupt number used for syscalls.
#define SYSCALL_INTERRUPT   49

enum syscall_num {
    // Process control
    SC_PROCESS_START,
    SC_PROCESS_EXIT,
    SC_PROCESS_PID,
    SC_PROCESS_PRIORITY,
    SC_PROCESS_SET_PRIORITY,
    SC_PROCESS_WAIT,
    SC_PROCESS_KILL,
    SC_PROCESS_WAIT_CLOCK,
    SC_PROCESS_NAME,
    SC_PROCESS_CHILDREN,
    // Process queue control
    SC_PROCESS_QUEUE_CREATE,
    SC_PROCESS_QUEUE_DELETE,
    SC_PROCESS_QUEUE_SEND,
    SC_PROCESS_QUEUE_RECEIVE,
    SC_PROCESS_QUEUE_COUNT,
    SC_PROCESS_QUEUE_RESET,
    // Clock settings
    SC_CLOCK_SETTINGS,
    SC_CLOCK_GET,
    // Console control
    SC_CONSOLE_WRITE,
    SC_CONSOLE_READ,
    SC_CONSOLE_ECHO,
    // Max number of syscalls
    SYSCALL_COUNT
};

#endif
