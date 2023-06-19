#include "stddef.h"
#include "stdio.h"

#include "internals.h"
#include "interrupt.h"
#include "process.h"
#include "syscall.h"
#include "pit.h"
#include "cga.h"
#include <stdint.h>


/// Function wrapper to check access rights to pointers.
static void clock_settings(uint32_t *quartz_freq, uint32_t *ticks) {
    if (process_check_user_ptr(quartz_freq) && process_check_user_ptr(ticks)) {
        pit_clock_settings(quartz_freq, ticks);
    }
}

/// Function wrapper for clock get.
static uint32_t clock_get() {
    return pit_clock_get();
}

/// Function wrapper for cons write to check access rights.
static void console_write(const char *s, int32_t len) {
    if (process_check_user_ptr(s)) {
        cga_write_bytes(s, len);
    }
}


/// Type alias for a syscall function handler.
typedef void *syscall_handler_t;

/// Define all syscall handlers, used by assembly.
syscall_handler_t syscall_handlers[SYSCALL_COUNT] = {
    [SC_PROCESS_START]          = process_start,
    [SC_PROCESS_EXIT]           = process_exit,
    [SC_PROCESS_PID]            = process_pid,
    [SC_PROCESS_NAME]           = process_name,
    [SC_PROCESS_PRIORITY]       = process_priority,
    [SC_PROCESS_SET_PRIORITY]   = process_set_priority,
    [SC_PROCESS_WAIT]           = process_wait,
    [SC_PROCESS_KILL]           = process_kill,
    [SC_PROCESS_WAIT_CLOCK]     = process_wait_clock,
    [SC_PROCESS_QUEUE_CREATE]   = process_queue_create,
    [SC_PROCESS_QUEUE_DELETE]   = process_queue_delete,
    [SC_PROCESS_QUEUE_SEND]     = process_queue_send,
    [SC_PROCESS_QUEUE_RECEIVE]  = process_queue_receive,
    [SC_PROCESS_QUEUE_COUNT]    = process_queue_count,
    [SC_PROCESS_QUEUE_RESET]    = process_queue_reset,
    [SC_CLOCK_SETTINGS]         = clock_settings,
    [SC_CLOCK_GET]              = clock_get,
    [SC_CONSOLE_WRITE]          = console_write,
    [SC_CONSOLE_READ]           = NULL,
    [SC_CONSOLE_ENABLE]         = NULL,
};

struct syscall_context *syscall_context = NULL;


/// Syscall interrupt handler defined in assembly.
void syscall_handler(void);

void syscall_init(void) {
    printf("Initializing syscall...\n");
    idt_interrupt_gate(SYSCALL_INTERRUPT, (uint32_t) syscall_handler, 3);
}
