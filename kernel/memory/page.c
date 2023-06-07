#include "memory.h"

#include "stdbool.h"
#include "stddef.h"
#include "string.h"
#include "stdio.h"
#include <stdint.h>

// Symbols defined in kernel.lds
extern uint8_t mem_heap;
extern uint8_t mem_heap_end;

static size_t heap_size = 0;
static size_t page_count = 0;
static size_t meta_page_count = 0;
static size_t alloc_count = 0;


// static bool page_is_allocated(size_t num) {
//     size_t index = num / 8;
//     size_t bit = num % 8;
//     uint8_t *slot = &mem_heap + index;
//     return (*slot & (1 << bit)) != 0;
// }

static void page_set_allocated(size_t num, bool allocated) {
    size_t index = num / 8;
    size_t bit = num % 8;
    uint8_t *slot = &mem_heap + index;
    if (allocated) {
        *slot |= 1 << bit;
    } else {
        *slot &= ~(1 << bit);
    }
}

static size_t page_count_padded(size_t size) {
    return (size - 1) / PAGE_SIZE + 1;
}


void page_init() {

    printf("Initializing page allocator...\n");

    heap_size = &mem_heap_end - &mem_heap;
    printf("Heap size: %d (%d Mio)\n", heap_size, heap_size / 1048576);

    // We split memory into blocks of 4K.
    page_count = heap_size / PAGE_SIZE;
    printf("Page count: %d (x %d)\n", page_count, PAGE_SIZE);

    // We use blocks where each bit indicates if a the corresponding
    // block is free, we can fit 8 blocks into one byte, so 
    // 8 * PAGE_SIZE in a single page.
    meta_page_count = page_count_padded(page_count);
    printf("Meta page count: %d\n", meta_page_count);

    // Clear meta pages.
    memset(&mem_heap, 0, meta_page_count * PAGE_SIZE);

    // Meta pages are marker as allocated, and will never be freed.
    for (size_t i = 0; i < meta_page_count; i++) {
        page_set_allocated(i, true);
    }

}

void *page_alloc(size_t size) {

    // assert(count > 0);

    size_t count = page_count_padded(size);

    // Compute the index/bit of the first non-meta page.
    size_t meta_index = meta_page_count / 8;
    size_t meta_bit = meta_page_count % 8;

    uint8_t *slot = &mem_heap + meta_index;
    uint8_t mask = 1 << meta_bit;
    size_t page_index = meta_page_count;

    size_t alloc_index = 0;
    size_t alloc_counter = 0;

    while (page_index < page_count) {

        // Page is not allocated.
        if ((*slot & mask) == 0) {

            if (alloc_counter == 0) {
                alloc_index = page_index;
            }

            if (++alloc_counter == count) {

                void *ptr = &mem_heap + (alloc_index * PAGE_SIZE);

                // Allocation successful.
                while (alloc_counter != 0) {
                    page_set_allocated(alloc_index++, true);
                    alloc_counter--;
                }

                alloc_count += count;

                return ptr;

            }

        } else {
            alloc_counter = 0;
        }

        // Move to the next bit, or the next slot if relevant.
        if (mask == 0x80) {
            slot++;
            mask = 1;
        } else {
            mask <<= 1;
        }

        page_index++;

    }

    return NULL;

}

void page_free(void *ptr, size_t size) {
    
    printf("page_free(%p, %d)\n", ptr, size);
    
    size_t count = page_count_padded(size);

    size_t page_offset = ptr - ((void *) &mem_heap);
    size_t page_index = page_offset / PAGE_SIZE;

    while (count != 0) {
        page_set_allocated(page_index++, false);
        count--;
    }

    alloc_count -= count;

}

size_t page_alloc_count() {
    return alloc_count;
}
