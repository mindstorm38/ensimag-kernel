/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include "mem_internals.h"
#include "memory.h"


static size_t power2(size_t size) {
    size_t p = 0;
    size = size - 1; // allocation start in 0
    while (size) {  // get the largest bit
        p++;
        size >>= 1;
    }
    if (size > ((size_t) 1 << p))
	    p++;
    return p;
}


// Internal function that ensures that the chunk size (2**index) is available 
// in its linked list.
static void alloc_tzl_chunk(size_t index) {

    if (arena.tzl[index]) {
        // A chunk already exists for this size.
        return;
    } else if (index >= FIRST_ALLOC_MEDIUM_EXPOSANT + arena.medium_next_exponant) {

        // It can happen that index we be greater than 'FIRST_ALLOC_MEDIUM_EXPOSANT + 1'
        // even with 'arena.medium_next_exponant == 0'. To account for this we adjust the
        // next exponent depending on the index. This is caused by +32 on size, for markers.
        //
        // This cannot happen once 'arena.medium_next_exponant == 1'.
        arena.medium_next_exponant = index - FIRST_ALLOC_MEDIUM_EXPOSANT;

        // We reached the next medium exponant, therefore we need to allocate a new block
        // at this particular index, therefore it exists and we can return.
        if (mem_realloc_medium() == 0) {

        }

        // The chunk we allocated has no next chunk.
        *((void **) arena.tzl[index]) = NULL;

        return;
    } else {

        // In this case we don't have any chunk at this level, so we split the next chunk.

        // The full size of the split chunks.
        size_t size = 1 << index;

        // Here we need to split the higher chunk into two chunks of the current size.
        alloc_tzl_chunk(index + 1);

        // Get the pointer to the two chunks, which is in the linked list of a greater size.
        void *chunk0 = arena.tzl[index + 1];
        void *chunk1 = (void *) ((size_t) chunk0 + size);

        // Remove the higher chunk from its linked list.
        void *higher_chunk_next = *((void **) chunk0);
        arena.tzl[index + 1] = higher_chunk_next;

        // Append the chunk to the head of the current linked list.
        void *chunk_next = arena.tzl[index];
        *((void **) chunk0) = chunk1;
        *((void **) chunk1) = chunk_next;
        arena.tzl[index] = chunk0;

    }

}

void *kalloc_medium(size_t size) {

    // assert(size < LARGEALLOC);
    // assert(size > SMALLALLOC);

    size_t size_marked = size + MARK_SIZE;
    size_t index = power2(size_marked); 
    // printf("size: %ld, full_size: %ld, index: %d\n", size, full_size, index);
    alloc_tzl_chunk(index);

    // Extract the chunk from the head of the linked list.
    void *alloc_chunk = arena.tzl[index];
    void *chunk_next = *((void **) alloc_chunk);
    arena.tzl[index] = chunk_next;

    return mem_mark_and_get_user_ptr(alloc_chunk, size_marked, MEM_MEDIUM);

}


// Internal function to free the given chunk, of a given size and at a given TZL index.
static void free_tzl_chunk(void *self_chunk, size_t size, size_t index) {

    // Compute the address of the buddy chunk.
    void *buddy_chunk = (void *) ((size_t) self_chunk ^ size);

    // Iterate over all chunks.
    void **chunk_it = &arena.tzl[index];
    while (*chunk_it) {
        if (*chunk_it == buddy_chunk) {

            // Buddy chunk found, we remove it from its linked list.
            // To do this, we get the chunk next to it.
            void *chunk_next = *((void **) buddy_chunk);
            *chunk_it = chunk_next;

            // Then, recursively free the chunk starting at the lowest pointer.
            if (buddy_chunk < self_chunk) {
                free_tzl_chunk(buddy_chunk, size << 1, index + 1);
            } else {
                free_tzl_chunk(self_chunk, size << 1, index + 1);
            }

            // Early-return to avoid added the current chunk to the current TZL.
            return;

        } else {
            chunk_it = (void **) *chunk_it;
        }
    }

    // No buddy chunk found, we add it to the current TZL's head.
    void *chunk_next = arena.tzl[index];
    *((void **) self_chunk) = chunk_next;
    arena.tzl[index] = self_chunk;

}

void kfree_medium(struct mem_alloc a) {
    size_t index = power2(a.size); 
    free_tzl_chunk(a.ptr, 1 << index, index);
}
