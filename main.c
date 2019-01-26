#include <xc.h>
#include <stdint.h>
#include "clock.h"
#include "can.h"
#include "frontend.h"
#include "elm327slcan.h"
#include "rbuf.h"

extern void init(void);

#define RSIZE   128
#define SSIZE   248

char    rBuf[RSIZE];
char    sBuf[SSIZE];

rbuf_t  rRing = { rBuf, 0, 0, RSIZE };
rbuf_t  sRing = { sBuf, 0, 0, SSIZE };

uint8_t state = STATE_CONFIG;

void __interrupt(high_priority) myIsr(void) {
    uint8_t data;
    
    if (UxTXIE && UxTXIF) {
        if (rbuf_pop_isr((rbuf_t *)&sRing, &data))
            UxTXREG = data;
        else
            cmdStop();
    }        
    
    if (UxRCIF && UxRCIE) {
        if (!rbuf_push_isr((rbuf_t *)&rRing, UxRCREG))
            SET_TX_OVFL();
    }        
}

int main(void) {
    uint8_t     ch, num, txtbuf[32];
    char        line[LINE_MAXLEN];
    uint8_t     linepos = 0;
    
    canmsg_t    canmsg_buffer;

    init();

    while (1) {
        if (state != STATE_CONFIG) {
            while (can_receive_message(&canmsg_buffer)) {
                num = canmsg2ascii(&canmsg_buffer, &txtbuf);
                if (num < rbuf_free_items((rbuf_t *)&sRing)) {
                    for (ch = 0; ch < num; ch++) {
                        rbuf_push((rbuf_t *)&sRing, txtbuf[ch]);
                    }    
                    cmdSend();
                }
                else SET_RX_OVFL();
            } 
        }       

        while (rbuf_pop((rbuf_t *)&rRing, &ch)) {
            if (ch == CR) {
                line[linepos] = 0;
                parseLine(line);
                linepos = 0;
            } else if (ch != LR) {
                line[linepos] = ch;
                if (linepos < LINE_MAXLEN - 1) linepos++;
            }
        }
    }
    return 0;
}
