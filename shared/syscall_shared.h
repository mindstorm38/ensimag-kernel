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

// Process control
#define SC_PROCESS_START            0
#define SC_PROCESS_EXIT             1
#define SC_PROCESS_PID              2
#define SC_PROCESS_NAME             3
#define SC_PROCESS_PRIORITY         4
#define SC_PROCESS_SET_PRIORITY     5
#define SC_PROCESS_WAIT             6
#define SC_PROCESS_KILL             7
#define SC_PROCESS_WAIT_CLOCK       8

// Process queue control
#define SC_PROCESS_QUEUE_CREATE     9
#define SC_PROCESS_QUEUE_DELETE     10
#define SC_PROCESS_QUEUE_SEND       11
#define SC_PROCESS_QUEUE_RECEIVE    12
#define SC_PROCESS_QUEUE_COUNT      13
#define SC_PROCESS_QUEUE_RESET      14

// Clock settings
#define SC_CLOCK_SETTINGS           15
#define SC_CLOCK_GET                16

// Console control
#define SC_CONSOLE_WRITE            17
#define SC_CONSOLE_READ             18
#define SC_CONSOLE_ENABLE           19

// Max number of syscalls
#define SYSCALL_COUNT               20

// Interrupt number used for syscalls.
#define SYSCALL_INTERRUPT   49

#endif
