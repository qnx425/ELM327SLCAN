#ifndef _CLOCK_
#define _CLOCK_

#include <stdint.h>

extern uint16_t clock_getMS(void);
extern void     clock_reset(void);

//#define CLOCK_TIMERTICKS_1MS 500
//#define CLOCK_TIMERTICKS_100MS 50000

#endif
