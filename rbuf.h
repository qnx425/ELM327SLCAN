#ifndef __RBUF_H_
#define __RBUF_H_

#include <stdint.h>

typedef struct {
    char    *buffer;
    uint8_t  head;
    uint8_t  tail;
    uint8_t  maxlen;
}   rbuf_t;

extern uint8_t rbuf_free_items(rbuf_t *);

/*
 * Method: rbuf_pop
 * Returns:
 *  1 - Success
 *  0 - Empty
 */
extern uint8_t rbuf_pop     (rbuf_t *, uint8_t *);
extern uint8_t rbuf_pop_isr (rbuf_t *, uint8_t *);

/*
 * Method: rbuf_push
 * Returns:
 *  1 - Success
 *  0 - Out of space
 */
extern uint8_t rbuf_push    (rbuf_t *, uint8_t);
extern uint8_t rbuf_push_isr(rbuf_t *, uint8_t);

#endif /* __RBUF_H_ */
