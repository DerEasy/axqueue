//
// Created by easy on 19.10.23.
//

#include "axqueue.h"
#include <stdlib.h>
#include <string.h>

#define MAX(x, y) ((x) > (y) ? (x) : (y))


struct axqueue {
    void **items;
    uint64_t front;
    uint64_t back;
    uint64_t len;
    uint64_t cap;
    void (*destroy)(void *);
};

static uint64_t toItemSize(uint64_t n) {
    return n * sizeof(void *);
}

axqueue *axq_newSized(uint64_t size) {
    size = MAX(1, size);
    axqueue *q = malloc(sizeof *q);
    if (q)
        q->items = malloc(size * sizeof *q->items);
    if (!q || !q->items) {
        free(q);
        return NULL;
    }
    q->first = 0;
    q->len = 0;
    q->cap = size;
    q->destroy = NULL;
    return q;
}

axqueue *axq_new(void) {
    return axq_newSized(7);
}

void axq_destroy(axqueue *q) {

}

void *axq_add(axqueue *q, void *val) {
    q->items[q->len++] = val;
}


