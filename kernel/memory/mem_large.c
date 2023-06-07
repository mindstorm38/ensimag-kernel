/******************************************************
 * Copyright Grégory Mounié 2018                      *
 * This code is distributed under the GLPv3+ licence. *
 * Ce code est distribué sous la licence GPLv3+.      *
 ******************************************************/

#include "memory.h"
#include "mem_internals.h"


void *kalloc_large(size_t size) {
	size_t size_marked = size + MARK_SIZE;
	void *mem = page_alloc(size_marked);
	if (!mem) {
		return NULL;
	}
	return mem_mark_and_get_user_ptr(mem, size_marked, MEM_LARGE);
}

void kfree_large(struct mem_alloc a) {
	page_free(a.ptr, a.size);
}
