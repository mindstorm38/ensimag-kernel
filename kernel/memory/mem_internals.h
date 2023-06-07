/// Internal functions for the kernel memory allocator.

#ifndef __MEM_INTERNALS_H__
#define __MEM_INTERNALS_H__

#include "stdbool.h"
#include "stdint.h"
#include "stddef.h"

// 2 MAGIC + 2 sizes
#define MARK_SIZE (sizeof(size_t) * 4)
#define MARK_USER_OFFSET (MARK_SIZE / 2)

#define SMALLALLOC 64
#define CHUNKSIZE (SMALLALLOC + MARK_SIZE)

// 128 Kio == 128 * 1024 == 2**17 == (1<<17)
#define LARGEALLOC (1 << 17) 

// 2**13o == 16Kio
#define FIRST_ALLOC_SMALL (CHUNKSIZE << 7) // 96o * 128
#define FIRST_ALLOC_MEDIUM_EXPOSANT 17
#define FIRST_ALLOC_MEDIUM (1 << FIRST_ALLOC_MEDIUM_EXPOSANT)

#define TZL_SIZE 48


enum mem_kind { MEM_SMALL, MEM_MEDIUM, MEM_LARGE };

struct mem_alloc {
    void *ptr;
    enum mem_kind kind;
    size_t size;
};

struct mem_arena {
    void *chunkpool;
    void *tzl[TZL_SIZE];
    uint32_t small_next_exponant;
    uint32_t medium_next_exponant;
};

extern struct mem_arena arena;

void *mem_mark_and_get_user_ptr(void *ptr, size_t size, enum mem_kind kind);
bool mem_mark_check(void *ptr, struct mem_alloc *a);

size_t mem_realloc_small();
size_t mem_realloc_medium();

void *kalloc_small(size_t size);
void *kalloc_medium(size_t size);
void *kalloc_large(size_t size);

void kfree_small(struct mem_alloc a);
void kfree_medium(struct mem_alloc a);
void kfree_large(struct mem_alloc a);

#endif
