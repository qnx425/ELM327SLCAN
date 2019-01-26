#include <xc.h>
#include <stdint.h>
#include "clock.h"
#include "elm327slcan.h"

extern uint8_t              state;

static volatile uint16_t    clock_msticker = 0, tmp;
static volatile uint8_t     led_ticker = 0;

void __interrupt(low_priority) myLoIsr(void) {
    if (CCP1IF && CCP1IE) {
        CCP1IF  = 0;
        // 1 ms
        if (++clock_msticker == 60000) clock_msticker = 0;
    }

    if (CCP2IF && CCP2IE) {
        CCP2IF  = 0;
        // 100 ms
        led_ticker = (led_ticker + 1) & 31;
        stateLED((led_ticker == 0) || (state != STATE_CONFIG));
        
        if (OERR()) {               // steady orange light
            errorLED(1);            // device must be repowered
        } else if (RXBnOVFL()) {    // one orange light flash
            errorLED((led_ticker & 2) && (led_ticker < 4));
        } else if (RX_OVFL())  {    // two orange light flash
            errorLED((led_ticker & 2) && (led_ticker < 8));
        } else if (TX_OVFL())  {    // three orange light flash
            errorLED((led_ticker & 2) && (led_ticker < 12));
        }    
    }
}

uint16_t clock_getMS(void) {
    CCP1IE = 0;
    tmp = clock_msticker;
    CCP1IE = 1;
    
    return tmp;
}

void clock_reset(void) {
    CCP1IE = 0;
    clock_msticker = 0;
    CCP1IE = 1;
}
