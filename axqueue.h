//
// Created by easy on 19.10.23.
//

#ifndef AXQUEUE_AXQUEUE_H
#define AXQUEUE_AXQUEUE_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/* simple and fast array-based queue (ring buffer) implementation */
typedef struct axqueue axqueue;

/**
 * Set custom memory functions. All three of them must be set and be compatible with one another.
 * Passing NULL for any function will activate its standard library counterpart.
 * @param malloc_fn The malloc function.
 * @param realloc_fn The realloc function.
 * @param free_fn The free function.
 */
void axq_memoryfn(void *(*malloc_fn)(size_t), void *(*realloc_fn)(void *, size_t), void (*free_fn)(void *));

/**
 * Unsigned length of queue.
 * @return Unsigned length.
 */
uint64_t axq_ulen(axqueue *q);

/**
 * Signed length of queue.
 * @return Signed length.
 */
int64_t axq_len(axqueue *q);

/**
 * Unsigned current capacity of queue, i.e., the number of items that could fit without resizing the underlying array.
 * @return Unsigned capacity.
 */
uint64_t axq_cap(axqueue *q);

/**
 * Unsigned limit of queue. The limit describes the queue's maximum capacity before it starts behaving like a ring
 * buffer.
 * @return Unsigned limit.
 */
uint64_t axq_getLimit(axqueue *q);

/**
 * Set the limit of this queue. The limit describes the queue's maximum capacity before it starts behaving like a ring
 * buffer. If the new limit subceeds the current capacity, the queue is resized.
 * @param limit Capacity limit.
 * @return True iff the resize operation failed (limit is still applied).
 */
bool axq_setLimit(axqueue *q, uint64_t limit);

/**
 * Get the currently stored destructor function of type void (*)(void *).
 * @return Destructor or NULL if not set.
 */
void (*axq_getDestructor(axqueue *q))(void *);

/**
 * Change the destructor function of type void (*)(void *).
 * @param destroy Destructor or NULL to disable.
 * @return Self.
 */
axqueue *axq_setDestructor(axqueue *q, void (*destroy)(void *));

/**
 * Get the context of this queue.
 * @return Context.
 */
void *axq_getContext(axqueue *q);

/**
 * Set some context for this queue.
 * @param context Context.
 * @return Self.
 */
axqueue *axq_setContext(axqueue *q, void *context);

/**
 * Determine the "ringiness" of this queue by checking if its length matches its limit. In that case, the queue is
 * considered full and adding additional items will overwrite the queue head (destructor called if set).
 * @return Whether length equals limit thereby causing ring behaviour to kick in.
 */
bool axq_isFull(axqueue *q);

/**
 * Create a new queue with an initial capacity and a custom limit.
 * @param size Initial capacity to allocate.
 * @param limit Capacity limit at which no further allocations shall take place.
 * @return Newly created queue or NULL iff OOM.
 */
axqueue *axq_newSized(uint64_t size, uint64_t limit);

/**
 * Create a new queue with default parameters. The limit is set to the largest possible integer UINT64_MAX.
 * @return Newly created queue or NULL iff OOM.
 */
axqueue *axq_new(void);

/**
 * Call the destructor on all items starting from the head if it is set; regardless, all memory is released and the
 * queue is invalidated thereafter.
 */
void axq_destroy(axqueue *q);

/**
 * Resize the queue's capacity. If the new capacity subceeds the current length of the queue, items starting from the
 * head are discarded until the queue content will fit the new capacity. Discarded items pass through the destructor if
 * one is set. The new capacity cannot exceed the queue's limit.
 * @param size The new capacity of the queue which is automatically locked to the queue's limit.
 * @return True iff OOM.
 */
bool axq_resize(axqueue *q, uint64_t size);

/**
 * Add an item to the queue. May resize the queue if needed.
 * @param value Item to add.
 * @return True iff OOM during resize operation.
 */
bool axq_add(axqueue *q, void *value);

/**
 * Pop off the head of the queue.
 * @return Head of queue or NULL if empty.
 */
void *axq_pop(axqueue *q);

/**
 * Take a peek at the head of the queue without removing it from the queue.
 * @return Head of queue or NULL if empty.
 */
void *axq_peek(axqueue *q);

/**
 * Let f be a function having the parameters (queue item, optional argument). Apply f to each item in the queue starting
 * from the head. Stop when reaching the end of the queue or when f returns false.
 * @param f Function of type bool (*)(void *, void *) to apply to the queue items.
 * @param args Optional argument passed to the function.
 * @return Self.
 */
axqueue *axq_foreach(axqueue *q, bool (*f)(void *, void *), void *args);

/**
 * Let f be a function having the parameters (queue item, optional argument). Apply f to each item in the queue starting
 * from the back. Stop when reaching the front of the queue or when f returns false.
 * @param f Function of type bool (*)(void *, void *) to apply to the queue items.
 * @param args Optional argument passed to the function.
 * @return Self.
 */
axqueue *axq_rforeach(axqueue *q, bool (*f)(void *, void *), void *args);

/**
 * Discard the first n items of the queue. Discarded items pass through the destructor if one is set.
 * @param n Number of items to discard.
 * @return Self.
 */
axqueue *axq_discard(axqueue *q, uint64_t n);

/**
 * Discard all items and pass them through the destructor if one is set.
 * @return Self.
 */
axqueue *axq_clear(axqueue *q);

/**
 * Create a copy of this queue. The destructor function is not copied along.
 * @return Newly created copy of queue or NULL iff OOM.
 */
axqueue *axq_copy(axqueue *q);

#endif //AXQUEUE_AXQUEUE_H
