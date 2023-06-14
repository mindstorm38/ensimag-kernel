/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include "memory.h"
#include "mem_internals.h"


void *kalloc_small(size_t size) {

    // We allocated blocks of fixed size.
    (void) size;

    if (arena.chunkpool == NULL) {
        
        unsigned long size = mem_realloc_small();
        if (size == 0)
            return NULL;
        
        void *chunk = arena.chunkpool;

        // This loops for N-1 chunks.
        for (unsigned long offset = CHUNKSIZE; offset < size; offset += CHUNKSIZE) {

            // Pointer to the next chunk.
            void *next_chunk = (void *) ((unsigned long) chunk + CHUNKSIZE);
            // The pointer is interpreted as a pointer to void*, we set this to the next chunk.
            *((void **) chunk) = next_chunk;
            chunk = next_chunk;

        }

        // The last chunk of the linked list contains no next chunk.
        *((void **) chunk) = NULL;

    }

    // Here we know that this is not null because we reallocated it.
    void *alloc_chunk = arena.chunkpool;
    void *new_head_chunk = *((void **) alloc_chunk);
    arena.chunkpool = new_head_chunk;

    return mem_mark_and_get_user_ptr(alloc_chunk, CHUNKSIZE, MEM_SMALL);

}

void kfree_small(struct mem_alloc a) {
    
    void *next_chunk = arena.chunkpool;
    arena.chunkpool = a.ptr;

    *((void **) arena.chunkpool) = next_chunk;

}
