#include "rbuf.h"

uint8_t rbuf_push(rbuf_t *c, uint8_t data) {
    uint8_t next;

    next = c->head + 1;         // next is where head will point to after this write.
    if (next >= c->maxlen)
        next = 0;

    if (next == c->tail)        // if the head + 1 == tail, circular buffer is full
        return 0;

    c->buffer[c->head] = data;  // Load data and then move
    c->head = next;             // head to next data offset.
    return 1;                   // return success to indicate successful push.
}

uint8_t rbuf_push_isr(rbuf_t *c, uint8_t data) {
    uint8_t next;

    next = c->head + 1;         // next is where head will point to after this write.
    if (next >= c->maxlen)
        next = 0;

    if (next == c->tail)        // if the head + 1 == tail, circular buffer is full
        return 0;

    c->buffer[c->head] = data;  // Load data and then move
    c->head = next;             // head to next data offset.
    return 1;                   // return success to indicate successful push.
}

uint8_t rbuf_pop(rbuf_t *c, uint8_t *data) {
    uint8_t next;

    if (c->head == c->tail)     // if the head == tail, we don't have any data
        return 0;

    next = c->tail + 1;         // next is where tail will point to after this read.
    if(next >= c->maxlen)
        next = 0;

    *data = c->buffer[c->tail]; // Read data and then move
    c->tail = next;             // tail to next offset.
    return 1;                   // return success to indicate successful push.
}

uint8_t rbuf_pop_isr(rbuf_t *c, uint8_t *data) {
    uint8_t next;

    if (c->head == c->tail)     // if the head == tail, we don't have any data
        return 0;

    next = c->tail + 1;         // next is where tail will point to after this read.
    if(next >= c->maxlen)
        next = 0;

    *data = c->buffer[c->tail]; // Read data and then move
    c->tail = next;             // tail to next offset.
    return 1;                   // return success to indicate successful push.
}

uint8_t rbuf_free_items(rbuf_t *c) {
    if (c->head >= c->tail) {
        uint8_t tmp = c->maxlen - c->head;
        return (tmp + c->tail);
    }
        
    return (c->tail - c->head);
}
