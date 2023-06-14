/******************************************************
 * Copyright Grégory Mounié 2018-2022                 *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include "stdint.h"
#include "stddef.h"
#include "stdio.h"

#include "memory.h"
#include "mem_internals.h"


struct mem_arena arena = {0};


static size_t hash_ptr(void *ptr) {
    return ((size_t) ptr) * 4294967279;
}

/// Ptr must be aligned to size_t bytes.
/// Size must be a multiple of size_t bytes, ideally.
void *mem_mark_and_get_user_ptr(void *ptr, size_t size, enum mem_kind  kind) {

    size_t magic = hash_ptr(ptr);
    magic &= ~((unsigned long) 0b11);
    magic |= kind;

    size_t *ptr_head = ptr;
    ptr_head[0] = size;
    ptr_head[1] = magic;

    // FIXME: must be aligned to pointer size for better performance
    size_t *ptr_tail = (size_t*) ((size_t) ptr + size - MARK_USER_OFFSET);
    ptr_tail[0] = magic;
    ptr_tail[1] = size;

    return ptr + MARK_USER_OFFSET;
    
}

bool mem_mark_check(void *ptr, struct mem_alloc *a) {

    size_t *ptr_head = (size_t*) (ptr - MARK_USER_OFFSET);
    size_t size = ptr_head[0];
    size_t magic = ptr_head[1];

    size_t *ptr_tail = (size_t*) ((size_t) ptr_head + size - MARK_USER_OFFSET);
    // assert(ptr_tail[0] == magic);
    // assert(ptr_tail[1] == size);

    if (ptr_tail[0] != magic || ptr_tail[1] != size) {
        return false;
    }

    a->ptr = ptr_head;
    a->kind = magic & 0b11;
    a->size = size;

    return true;

}


size_t mem_realloc_small() {

    // assert(arena.chunkpool == 0);

    size_t size = (FIRST_ALLOC_SMALL << arena.small_next_exponant);
    arena.chunkpool = page_alloc(size);

    if (!arena.chunkpool) {
        return 0;
    }

    arena.small_next_exponant++;
    return size;
    
}

size_t mem_realloc_medium() {

    uint32_t exp = FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant;

    // assert(arena.tzl[exp] == 0);
    size_t size = (FIRST_ALLOC_MEDIUM << arena.medium_next_exponant);
    // assert(size == (1UL << exp));

    // Twice the size for alignment.
    arena.tzl[exp] = page_alloc(size * 2);

    if (!arena.tzl[exp]) {
        return 0;
    }

    // align allocation to a multiple of the size
    // for buddy algo
    arena.tzl[exp] += (size - (((size_t) arena.tzl[exp]) % size));
    
    arena.medium_next_exponant++;
    return size; // lie on allocation size, but never free

}
