/// User Mode and Syscall internal functions.

#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#include "syscall_shared.h"
#include "stddef.h"

size_t syscall0(size_t num);
size_t syscall1(size_t num, size_t p0);
size_t syscall2(size_t num, size_t p0, size_t p1);
size_t syscall3(size_t num, size_t p0, size_t p1, size_t p2);
size_t syscall4(size_t num, size_t p0, size_t p1, size_t p2, size_t p3);
size_t syscall5(size_t num, size_t p0, size_t p1, size_t p2, size_t p3, size_t p4);

#endif
