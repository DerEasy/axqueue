//
// Created by easy on 19.10.23.
//

#include "axqueue.h"
#include <stdlib.h>
#include <string.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define BETWEEN(x, y, z) (MIN(MAX((x), (y)), (z)))
#define ifwhile(p, q) if (p) while (q)

static void *(*malloc_)(size_t size) = malloc;
static void *(*realloc_)(void *ptr, size_t size) = realloc;
static void (*free_)(void *ptr) = free;

struct axqueue {
    void **items;
    uint64_t front;  /* always points to current head of queue */
    uint64_t back;   /* always points to where last value has been written */
    uint64_t len;
    uint64_t cap;
    uint64_t maxcap;
    void (*destroy)(void *);
};

static uint64_t toItemSize(uint64_t n) {
    return n * sizeof(void *);
}

uint64_t axq_ulen(axqueue *q) {
    return q->len;
}

int64_t axq_len(axqueue *q) {
    return q->len;
}

axqueue *axq_newSized(uint64_t size, uint64_t maxSize) {
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
    q->maxcap = maxSize;
    q->destroy = NULL;
    return q;
}

axqueue *axq_new(void) {
    return axq_newSized(7, UINT64_MAX);
}

void axq_destroy(axqueue *q) {
    if (q->destroy) while (q->len--) {
        q->destroy(q->items[q->front++]);
        q->front *= q->front < q->cap;
    }
    free_(q->items);
    free_(q);
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
    if (shift < 0)
        shift = q->cap + shift;
    q->front += shift;
    q->back += shift;
    q->front -= (q->front >= q->cap) * q->cap;
    q->back -= (q->back >= q->cap) * q->cap;
    reverseSection(q, 0, q->cap);
    reverseSection(q, 0, shift);
    reverseSection(q, shift, q->cap);
    return q;
}

bool axq_resize(axqueue *q, uint64_t size) {
    size = BETWEEN(1, size, q->maxcap);
    if (size > q->cap) {
        void **items = realloc_(q->items, toItemSize(size));
        if (!items)
            return true;
        if (q->back < q->front) {
            uint64_t frontToEnd = q->cap - q->front;
            uint64_t startToBack = q->back + 1;
            if (startToBack < frontToEnd) {
                uint64_t addedCap = size - q->cap;
                uint64_t toCopy = MIN(startToBack, addedCap);
                memmove(&items[q->cap], &items[startToBack - toCopy], toItemSize(toCopy));
                q->back += toCopy == startToBack ? q->cap : -toCopy;
            } else {
                uint64_t moveBy = size - q->cap;
                memmove(&items[q->front + moveBy], &items[q->front], toItemSize(frontToEnd));
                q->front += moveBy;
            }
        }
        q->items = items;
        q->cap = size;
    } else if (size < q->cap) {
        uint64_t discard = (size < q->len) * (q->len - size);
        if (q->destroy) while (discard--) {
            q->destroy(q->items[q->front++]);
            q->front *= q->front >= q->cap;
        } else {
            q->front += discard;
            q->front -= (q->front >= q->cap) * q->cap;
        }
        axq_rotate(q, -q->front);
        q->len -= discard;
        void **items = realloc_(q->items, toItemSize(size));
        if (!items)
            return true;
        q->items = items;
        q->cap = size;
    }
    return false;
}

bool axq_add(axqueue *q, void *value) {
    if (q->len >= q->cap && axq_resize(q, (q->cap << 1) | 1))
        return true;
    if (q->len == q->cap) {
        q->front = (++q->back, q->back *= q->back < q->cap);
        if (q->destroy)
            q->destroy(q->items[q->back]);
    } else {
        q->back += !!q->len;
        q->back *= q->back < q->cap;
    }
    q->items[q->back] = value;
    ++q->len;
    return false;
}

void *axq_pop(axqueue *q) {
    void *value = NULL;
    if (q->len) {
        value = q->items[q->front++];
        if (--q->len)
            q->front *= q->front < q->cap;
        else
            q->back = q->front = 0;
    }
    return value;
}
