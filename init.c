#include <xc.h>
#include <stdint.h>
#include "elm327slcan.h"

void init(void) {
    LATA      = 0;
#if defined(__DEBUG)
    TRISA    &= 0b11010000;
#endif
    ANCON0    = 0;              // Digital port
    ANCON1   &= 0xF8;           // Digital port
    
    // EUSART init
    UxBAUDCON = 0b01001000;
    UxSPBRGH  = BRGVALUE >>  8;
    UxSPBRG   = BRGVALUE & 255;
    UxTXSTA   = 0b00100110;
    UxRCSTA   = 0b10010000;

    // ECAN init
#if defined(__DEBUG)
    LATC      = 0x40;           // RC6 as CANTX output
    TRISC     = 0x80;           // RC7 as CANRX input
    TRISB6    = 0;
#else
    LATB      = 0b11000100;     // RB2 as CANTX output
    TRISB     = 0b00111011;     // RB3 as CANRX input
    TRISC6    = 0;
#endif
    CANCON    = 0x80;           // Configuration mode
    while ((CANSTAT & 0xE0) != 0x80) continue; 
    BSEL0     = 0;              // Buffer 5 to 0 are configured in Receive mode
//    BRGCON1  = 1;             // Tq = 1/4 us,  SJW = 1
//    BRGCON2  = 0b10001010;    // SEG1PH = 2, PRSEG = 3
//    BRGCON3  = 1;             // SEG2PH = 2
    CIOCON    = 0b00100000;
    ECANCON   = 0b10110000;     // Mode 2

    RXM0SIDH  = 0xFF;
    //RXM0SIDL  = 0xEB;
    RXM0SIDL  = 0xE3;
    RXM0EIDH  = 0xFF;
    RXM0EIDL  = 0xFF;
    RXM1SIDH  = 0xFF;
    //RXM1SIDL  = 0xEB;
    RXM1SIDL  = 0xE3;
    RXM1EIDH  = 0xFF;
    RXM1EIDL  = 0xFF;
 
    uint8_t i, *ptr;

    i   = 24;
    ptr = &RXF0SIDH;
    do {
        *ptr++ = 0;
    }   while (--i);
    
    i   = 40;
    ptr = &RXF6SIDH;
    do {
        *ptr++ = 0;
    }   while (--i);
    
    IPEN     = 1;
    
    // clock
    CCP1IP   = 0;
    CCPR1H   = 0x0F;
    CCPR1L   = 0xA0;
    CCP1CON  = 0x0B;
    CCP1IE   = 1;
    
    CCP2IP   = 0;
    CCPR2H   = 0xC3;
    CCPR2L   = 0x50;
    CCP2CON  = 0x0B;
    C2TSEL   = 1;
    CCP2IE   = 1;
            
    T1CON    = 1;
    T3CON    = 0x31;
    
    // uart
    UxRCIE   = 1;
    
    stateLED(0);

    INTCON   = 0xC0;
}
