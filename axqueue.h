//
// Created by easy on 19.10.23.
//

#ifndef AXQUEUE_AXQUEUE_H
#define AXQUEUE_AXQUEUE_H

#include <stdbool.h>

// simple and fast array-based queue implementation
typedef struct AXqueue AXqueue;

typedef struct AXqueueFuncs {
    // create new queue with default capacity
    AXqueue *(*new)(void);
    // create new queue with custom capacity
    AXqueue *(*sizedNew)(unsigned long size);
    // destroy queue and optionally free all items
    void (*destroy)(AXqueue *q);
    // put value in queue; returns true iff OOM
    bool (*enqueue)(AXqueue *q, void *val);
    // remove and return first value in queue or just return NULL if empty
    void *(*dequeue)(AXqueue *q);
    // return first value in queue or NULL if empty
    void *(*front)(AXqueue *q);
    // get length of queue; never negative
    long (*len)(AXqueue *q);
    // index queue; 0 first, -1 last in queue etc.; returns NULL on index error
    void *(*at)(AXqueue *q, long index);
    // swap two items; 0 first, -1 last in queue etc.; return true iff index error
    bool (*swap)(AXqueue *q, long index1, long index2);
    // reverse queue order in-place; returns queue
    AXqueue *(*reverse)(AXqueue *q);
    // remove all items, optionally freeing them; returns queue
    AXqueue *(*clear)(AXqueue *q);
    // create new shallow copy of queue
    AXqueue *(*copy)(AXqueue *q);
    // set capacity to some value thereby calling the destructor on excess items when shrinking. True iff OOM,
    // changing length of queue and calling of destructors is done regardless of fail or not
    bool (*resize)(AXqueue *q, unsigned long size);
    // call destructor if available on item
    AXqueue *(*destroyItem)(AXqueue *q, void *val);
    // set destructor function (passing NULL will disable destructor)
    AXqueue *(*setDestructor)(AXqueue *q, void (*destroy)(void *));
    // get destructor function
    void (*(*getDestructor)(AXqueue *q))(void *);
    // get pointer to underlying array; points to first in queue
    void **(*data)(AXqueue *q);
    // get capacity of queue; never negative
    long (*cap)(AXqueue *q);
} AXqueueFuncs;

extern const AXqueueFuncs axq;

#endif //AXQUEUE_AXQUEUE_H
