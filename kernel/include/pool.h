/// Generic ID pool.

#ifndef __POOL_H__
#define __POOL_H__

#include "stddef.h"


struct id_pool_slot {
    int id;
    struct id_pool_slot *next;
};

#define id_pool_t(cap) struct { \
    size_t next_id; \
    size_t counter; \
    struct id_pool_slot *free; \
    struct id_pool_slot slots[cap]; \
}

/// Internal function to pop an ID pool slot.
static inline int id_pool_pop_free(struct id_pool_slot **free, size_t *counter) {
    if (*free == NULL) {
        return -1;
    } else {
        (*counter)++;
        struct id_pool_slot *slot = *free;
        *free = slot->next;
        return slot->id;
    }
}

/// Internal function to pop a new ID slot, because it is new, we set
/// its id.
static inline int id_pool_pop_new(struct id_pool_slot *slot, int id, size_t *counter) {
    (*counter)++;
    slot[id].id = id;
    return id;
}

static inline void id_pool_push_free(struct id_pool_slot **free, struct id_pool_slot *slot, size_t *counter) {
    slot->next = *free;
    *free = slot;
    (*counter)--;
}

#define id_pool_cap(var) (sizeof(var.slots) / sizeof(var.slots[0]))
#define id_pool_empty(var) (var.counter >= id_pool_cap(var))

#define id_pool_alloc(var) (var.next_id >= id_pool_cap(var) ? id_pool_pop_free(&var.free, &var.counter) : id_pool_pop_new(var.slots, var.next_id++, &var.counter))
#define id_pool_free(var, id) id_pool_push_free(&var.free, &var.slots[id], &var.counter)

#endif 
