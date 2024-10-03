//
// Created by easy on 19.10.23.
//

#ifndef AXQUEUE_AXQUEUE_H
#define AXQUEUE_AXQUEUE_H

#include <stdbool.h>
#include <stdint.h>

// simple and fast array-based queue (ring buffer) implementation
typedef struct axqueue axqueue;

uint64_t axq_ulen(axqueue *q);

int64_t axq_len(axqueue *q);

uint64_t axq_cap(axqueue *q);

uint64_t axq_limit(axqueue *q);

void (*axq_getDestructor(axqueue *q))(void *);

axqueue *axq_setDestructor(axqueue *q, void (*destroy)(void *));

bool axq_isFull(axqueue *q);

axqueue *axq_newSized(uint64_t size, uint64_t limit);

axqueue *axq_new(void);

void axq_destroy(axqueue *q);

bool axq_resize(axqueue *q, uint64_t size);

bool axq_add(axqueue *q, void *value);

void *axq_pop(axqueue *q);

void *axq_peek(axqueue *q);

axqueue *axq_foreach(axqueue *q, bool (*f)(void *, void *), void *args);

axqueue *axq_rforeach(axqueue *q, bool (*f)(void *, void *), void *args);

axqueue *axq_discard(axqueue *q, uint64_t n);

axqueue *axq_clear(axqueue *q);

axqueue *axq_copy(axqueue *q);

#endif //AXQUEUE_AXQUEUE_H
