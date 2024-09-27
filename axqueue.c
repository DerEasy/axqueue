//
// Created by easy on 19.10.23.
//

#include "axqueue.h"
#include <stdlib.h>
#include <string.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))

static void *(*malloc_)(size_t size) = malloc;
static void *(*realloc_)(void *ptr, size_t size) = realloc;
static void (*free_)(void *ptr) = free;

struct axqueue {
    void **items;
    uint64_t front;  /* always points to current head of queue */
    uint64_t back;   /* always points to where next value will be written */
    uint64_t len;
    uint64_t cap;
    void (*destroy)(void *);
    bool ring;
};

static uint64_t toItemSize(uint64_t n) {
    return n * sizeof(void *);
}

axqueue *axq_newSized(uint64_t size) {
    size = MAX(1, size);
    axqueue *q = malloc_(sizeof *q);
    if (q)
        q->items = malloc_(toItemSize(size));
    if (!q || !q->items) {
        free(q);
        return NULL;
    }
    q->front = 0;
    q->back = 0;
    q->len = 0;
    q->cap = size;
    q->destroy = NULL;
    q->ring = false;
    return q;
}

axqueue *axq_new(void) {
    return axq_newSized(7);
}

void axq_destroy(axqueue *q) {

}

static void reverseSection(axqueue *q, uint64_t start, uint64_t end) {
    while (start < end--) {   /* end-- comes here so end is treated exclusively */
        void *tmp = q->items[start];
        q->items[start++] = q->items[end];
        q->items[end] = tmp;
    }
}

axqueue *axq_rotate(axqueue *q, int64_t shift) {
    shift %= (int64_t) q->len;
    if (shift == 0)
        return q;
    reverseSection(q, 0, q->cap);
    reverseSection(q, 0, shift);
    reverseSection(q, shift, q->cap);
    return q;
}

bool axq_resize(axqueue *q, uint64_t size) {
    size = MAX(1, size);
    if (size > q->cap) {
        void **items = realloc_(q->items, toItemSize(size));
        if (!items)
            return true;
        if (q->front >= q->back) {
            uint64_t frontToEnd = q->cap - q->front;
            uint64_t moveBy = size - q->cap;
            memmove(&items[q->front + moveBy], &items[q->front], toItemSize(frontToEnd));
            q->front += moveBy;
        }
        q->items = items;
        q->cap = size;
    } else if (size < q->cap) {
        uint64_t discard = (size < q->len) * (q->len - size);
        axq_rotate(q, -q->front - discard);
        q->front = discard ? q->cap - discard : 0;
        q->back = discard ? q->len - discard : q->len;
        q->len -= discard;
        while (q->destroy && q->front) {
            q->destroy(q->items[q->front++]);
            if (q->front >= q->cap)
                q->front = 0;
        }
        void **items = realloc_(q->items, toItemSize(size));
        if (!items)
            return true;
        q->items = items;
        q->cap = size;
    }
    return false;
}

bool axq_add(axqueue *q, void *value) {
    if (q->ring) {
        q->items[q->back] = value;
        q->len += q->front != q->back;
        q->front += q->front == q->back;
        ++q->back;
    } else {
        if (q->len >= q->cap && axq_resize(q, (q->cap << 1) | 1))
            return true;
        q->items[q->back++] = value;
        ++q->len;
    }
    return false;
}


