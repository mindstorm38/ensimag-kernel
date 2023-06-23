/// Kernel memory allocation.

#ifndef __MEMORY_H__
#define __MEMORY_H__

#include "stddef.h"

#define PAGE_SIZE 4096


/// Initialize the memory allocation system of the kernel. This must
/// be called once on startup, before everything.
void page_init();
/// Low-level kernel page allocation function, return null if failing.
/// The allocated pointer is aligned to the page size.
void *page_alloc(size_t size);
/// Low-level kernel page free function, pointer and count are not 
/// checked to be valid, be careful.
void page_free(void *ptr, size_t size);

/// Return the total number of page allocatable.
size_t page_capacity(void);
/// Get the current allocation count.
size_t page_used(void);

// TODO: Alignment guarantees.
void *kalloc(size_t size);
/// Free a pointer allocated with 'kalloc'.
void kfree(void *ptr);

#endif
