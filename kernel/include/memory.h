/// Kernel memory allocation.

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
/// Get the current allocation count.
size_t page_alloc_count();

void *kalloc(size_t size);
void kfree(void *ptr);
