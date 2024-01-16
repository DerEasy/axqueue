//
// Created by easy on 19.10.23.
//

#ifndef AXQUEUE_AXQUEUE_H
#define AXQUEUE_AXQUEUE_H

#include <stdbool.h>

// simple and fast array-based queue implementation
typedef struct axqueue axqueue;

struct axqueueFn {
    // create new queue with default capacity
    axqueue *(*new)(void);
    // create new queue with custom capacity
    axqueue *(*sizedNew)(unsigned long size);
    // destroy queue and optionally free all items
    void (*destroy)(axqueue *q);
    // put value in queue; returns true iff OOM
    bool (*enqueue)(axqueue *q, void *val);
    // remove and return first value in queue or just return NULL if empty
    void *(*dequeue)(axqueue *q);
    // return first value in queue or NULL if empty
    void *(*front)(axqueue *q);
    // get length of queue; never negative
    long (*len)(axqueue *q);
    // index queue; 0 first, -1 last in queue etc.; returns NULL on index error
    void *(*at)(axqueue *q, long index);
    // swap two items; 0 first, -1 last in queue etc.; return true iff index error
    bool (*swap)(axqueue *q, long index1, long index2);
    // reverse queue order in-place; returns queue
    axqueue *(*reverse)(axqueue *q);
    // remove all items, optionally freeing them; returns queue
    axqueue *(*clear)(axqueue *q);
    // create new shallow copy of queue
    axqueue *(*copy)(axqueue *q);
    // set capacity to some value thereby calling the destructor on excess items when shrinking. True iff OOM,
    // changing length of queue and calling of destructors is done regardless of fail or not
    bool (*resize)(axqueue *q, unsigned long size);
    // call destructor if available on item
    axqueue *(*destroyItem)(axqueue *q, void *val);
    // set destructor function (passing NULL will disable destructor)
    axqueue *(*setDestructor)(axqueue *q, void (*destroy)(void *));
    // get destructor function
    void (*(*getDestructor)(axqueue *q))(void *);
    // get pointer to underlying array; points to first in queue
    void **(*data)(axqueue *q);
    // get capacity of queue; never negative
    long (*cap)(axqueue *q);
};

#ifdef AXQUEUE_NAMESPACE
#define axq AXQUEUE_NAMESPACE
#endif

extern const struct axqueueFn axq;

#undef axq

#endif //AXQUEUE_AXQUEUE_H
