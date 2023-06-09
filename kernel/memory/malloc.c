#include "memory.h"
#include "mem_internals.h"

#include "stdio.h"


void *kalloc(size_t size) {

    if (size <= 0)
        return NULL;

    
    if (size >= LARGEALLOC)
	    return kalloc_large(size);
    else if (size <= SMALLALLOC)
	    return kalloc_small(size);
    else
	    return kalloc_medium(size);

}

void kfree(void *ptr) {

    struct mem_alloc a;

    if (!mem_mark_check(ptr, &a)) {
        return;
    }

    switch (a.kind) {
        case MEM_SMALL:
            kfree_small(a);
            break;
        case MEM_MEDIUM:
            kfree_medium(a);
            break;
        case MEM_LARGE:
            kfree_large(a);
            break;
    }

}
