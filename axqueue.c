//
// Created by easy on 19.10.23.
//

#include "axqueue.h"
#include <stdlib.h>
#include <string.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define BETWEEN(x, y, z) (MIN(MAX((x), (y)), (z)))

static void *(*malloc_)(size_t size) = malloc;
static void *(*realloc_)(void *ptr, size_t size) = realloc;
static void (*free_)(void *ptr) = free;

struct axqueue {
    void **items;
    uint64_t front;  /* always points to current head of queue */
    uint64_t back;   /* always points to where last value has been written */
    uint64_t len;
    uint64_t cap;
    uint64_t limit;
    void (*destroy)(void *);
    void *context;
};

static uint64_t toItemSize(uint64_t n) {
    return n * sizeof(void *);
}

void axq_memoryfn(void *(*malloc_fn)(size_t), void *(*realloc_fn)(void *, size_t), void (*free_fn)(void *)) {
    malloc_ = malloc_fn ? malloc_fn : malloc;
    realloc_ = realloc_fn ? realloc_fn : realloc;
    free_ = free_fn ? free_fn : free;
}

uint64_t axq_ulen(axqueue *q) {
    return q->len;
}

int64_t axq_len(axqueue *q) {
    return q->len;
}

uint64_t axq_cap(axqueue *q) {
    return q->cap;
}

uint64_t axq_getLimit(axqueue *q) {
    return q->limit;
}

bool axq_setLimit(axqueue *q, uint64_t limit) {
    return (q->limit = limit) < q->cap && axq_resize(q, limit);
}

void (*axq_getDestructor(axqueue *q))(void *) {
    return q->destroy;
}

axqueue *axq_setDestructor(axqueue *q, void (*destroy)(void *)) {
    q->destroy = destroy;
    return q;
}

void *axq_getContext(axqueue *q) {
    return q->context;
}

axqueue *axq_setContext(axqueue *q, void *context) {
    q->context = context;
    return q;
}

bool axq_isFull(axqueue *q) {
    return q->len == q->limit;
}

axqueue *axq_newSized(uint64_t size, uint64_t limit) {
    size = BETWEEN(1, size, limit);
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
    q->limit = limit;
    q->destroy = NULL;
    q->context = NULL;
    return q;
}

axqueue *axq_new(void) {
    return axq_newSized(7, UINT64_MAX);
}

void *axq_destroy(axqueue *q) {
    if (q->destroy) while (q->len--) {
        q->destroy(q->items[q->front++]);
        q->front *= q->front < q->cap;
    }
    void *context = q->context;
    free_(q->items);
    free_(q);
    return context;
}

static void reverseSection(axqueue *q, uint64_t start, uint64_t end) {
    while (start < end--) {   /* end-- comes here so end is treated exclusively */
        void *tmp = q->items[start];
        q->items[start++] = q->items[end];
        q->items[end] = tmp;
    }
}

static axqueue *preservativeRotation(axqueue *q, int64_t shift) {
    if (q->len == 0 || (shift %= (int64_t) q->len) == 0)
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
    size = BETWEEN(1, size, q->limit);
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
        preservativeRotation(q, -q->front);
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
        if (q->destroy)
            q->destroy(q->items[q->front]);
        ++q->front, q->front *= q->front < q->cap;
        ++q->back, q->back *= q->back < q->cap;
    } else {
        q->back += !!q->len;
        q->back *= q->back < q->cap;
        ++q->len;
    }
    q->items[q->back] = value;
    return false;
}

void *axq_pop(axqueue *q) {
    void *value = NULL;
    if (q->len) {
        value = q->items[q->front++];
        if (--q->len)
            q->front *= q->front < q->cap;
        else
            q->front = q->back = 0;
    }
    return value;
}

void *axq_peek(axqueue *q) {
    return q->len ? q->items[q->front] : NULL;
}

axqueue *axq_foreach(axqueue *q, bool (*f)(void *, void *), void *args) {
    uint64_t i = q->front;
    uint64_t n = q->len;
    while (n-- && f(q->items[i++], args))
        i *= i < q->cap;
    return q;
}

axqueue *axq_rforeach(axqueue *q, bool (*f)(void *, void *), void *args) {
    uint64_t i = q->back;
    uint64_t n = q->len;
    while (n-- && f(q->items[i--], args)) {
        if (i >= q->cap)
            i = q->cap - 1;
    }
    return q;
}

axqueue *axq_discard(axqueue *q, uint64_t n) {
    if (n == 0)
        return q;
    n = MIN(n, q->len);
    if (q->destroy) while (n--) {
        q->destroy(q->items[q->front++]);
        q->front *= q->front < q->cap;
    } else {
        uint64_t firstPortion = MIN(q->cap - q->front, n);
        uint64_t secondPortion = n - firstPortion;
        q->front += firstPortion;
        q->front *= q->front < q->cap;
        q->front += secondPortion;
    }
    if (q->len == 0)
        q->front = q->back = 0;
    return q;
}

axqueue *axq_clear(axqueue *q) {
    if (q->destroy) while (q->len--) {
        q->destroy(q->items[q->front++]);
        q->front *= q->front < q->cap;
    }
    q->front = q->back = q->len = 0;
    return q;
}

axqueue *axq_copy(axqueue *q) {
    axqueue *q2 = axq_newSized(q->cap, q->limit);
    if (!q2)
        return NULL;
    if (q->front <= q->back) {
        memcpy(q2->items, &q->items[q->front], toItemSize(q->len));
    } else {
        uint64_t frontToEnd = q->cap - q->front;
        uint64_t startToBack = q->back + 1;
        memcpy(q2->items, &q->items[q->front], toItemSize(frontToEnd));
        memcpy(&q2->items[frontToEnd], q->items, toItemSize(startToBack));
    }
    q2->len = q->len;
    q2->back = q->len - !!q->len;
    return q2;
}
