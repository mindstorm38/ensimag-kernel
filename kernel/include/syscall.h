/// User Mode and Syscall internal functions.

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "syscall_shared.h"
#include <stdint.h>

/// Initialize the syscall interface by setting the interrupt handler
/// for syscall interrupt (49).
void syscall_init(void);

#endif
