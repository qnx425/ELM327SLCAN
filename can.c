#include <xc.h>
#include "elm327slcan.h"
#include "clock.h"
#include "can.h"

/**
 * \brief Write to given register
 *
 * \param address Register address
 * \param data Value to write to given register
 */
void can_write_register(unsigned short address, unsigned char data) {
   FSR0  = address;
   INDF0 = data;
}

/**
 * \brief Read from given register
 *
 * \param address Register address
 * \return register value
 */
unsigned char can_read_register(unsigned short address) {
   FSR0 = address;
    
   return INDF0;
}

/**
 * \brief Set filter mask of given SJA1000 register values
 *
 * \param amr0 Acceptence mask register 0
 * \param amr1 Acceptence mask register 1
 * \param amr2 Acceptence mask register 2
 * \param amr3 Acceptence mask register 3
 *
 * This function has only affect if can controller is in configuration mode.
 */
void can_set_SJA1000_filter_mask(unsigned char amr0, unsigned char amr1, unsigned char amr2, unsigned char amr3) {

    // SJA1000   mask bit definition: 1 = accept without matching, 0 = do matching with acceptance code
    // Microchip mask bit definition: 0 = accept without matching, 1 = do matching with acceptance filter
    // -> invert mask

    RXM0SIDH = ~amr0;
    RXM0SIDL = ((~amr1) & 0xE0) | (((~amr1) >> 3) & 3);
    if (amr3 & 1) RXM0SIDL |= 8;
    RXM0EIDH = ((~amr2) >> 3) | ((~amr1) << 5);
    RXM0EIDL = ((~amr3) >> 3) | ((~amr2) << 5);
/*
    // mask for filter 1
    RXM0SIDH = ~amr0;
    RXM0SIDL = ((~amr1) & 0xE3) | 8;
    RXM0EIDH = ~amr2;
    RXM0EIDL = ~amr3;

    // mask for filter 2
    RXM1SIDH = ~amr2;
    RXM1SIDL = (~amr3) & 0xE0;
    RXM1EIDH = 0x00;
    RXM1EIDL = 0x00;
*/
}

/**
 * \brief Set filter code of given SJA1000 register values
 *
 * \param amr0 Acceptence code register 0
 * \param amr1 Acceptence code register 1
 * \param amr2 Acceptence code register 2
 * \param amr3 Acceptence code register 3
 *
 * This function has only affect if controller is in configuration mode.
 */
void can_set_SJA1000_filter_code(unsigned char acr0, unsigned char acr1, unsigned char acr2, unsigned char acr3) {
    RXF0SIDH = acr0;
    RXF0SIDL = (acr1 & 0xE0) | ((acr1 >> 3) & 3);
    if (acr3 & 1) RXF0SIDL |= 8;
    RXF0EIDH = (acr2 >> 3) | (acr1 << 5);
    RXF0EIDL = (acr3 >> 3) | (acr2 << 5);
/*
    // acceptance code for filter 1
    RXF0SIDH = acr0;
    RXF0SIDL = (acr1) & 0xE0; // standard
    RXF1SIDH = acr0;
    RXF1SIDL = ((acr1) & 0xE0) | 0x08; // extended

    // acceptance code for filter 2
    RXF2SIDH =  acr2;
    RXF2SIDL = (acr3) & 0xE0; // standard
    RXF3SIDH = acr2;
    RXF3SIDL = ((acr3) & 0xE0) | 0x08; // extended

    // fill remaining filters with zero
//    RXF4SIDH = 0x00;
//    RXF4SIDL = 0x00;
//    RXF5SIDH = 0x00;
//    RXF5SIDL = 0x00;
*/
}

/**
 * \brief Set bit timing registers
 *
 * \param cnf1 Configuration register 1
 * \param cnf2 Configuration register 2
 * \param cnf3 Configuration register 3
 *
 * This function has only affect if controller is in configuration mode
 */
void can_set_bittiming(unsigned char cnf1, unsigned char cnf2, unsigned char cnf3) {

    BRGCON1 = cnf1;
    BRGCON2 = cnf2;
    BRGCON3 = cnf3;
}

/**
 * \brief Send given CAN message
 *
 * \ p_canmsg Pointer to can message to send
 * \return 1 if transmitted successfully to transmit buffer, 0 on error (= no free buffer available)
 */
unsigned char can_send_message(canmsg_t * p_canmsg) {
    ECANCON &= 0xE0;
    ECANCON |= 0x05;
    
    if (RXB0CON & 0x08) {
        ECANCON--;
        if (RXB0CON & 0x08) {
            ECANCON--;
            if (RXB0CON & 0x08) {
                return 0;   //All TX buffers are busy
            }
        }
    }
    
    unsigned char length = p_canmsg->dlc;
    if (length > 8) length = 8;
        
    if (p_canmsg->flags.extended) {
        RXB0SIDH = p_canmsg->id >> 21;
        RXB0SIDL = ((p_canmsg->id >> 13) & 0xe0) | ((p_canmsg->id >> 16) & 0x03) | 0x08;
        RXB0EIDH = p_canmsg->id >> 8;
        RXB0EIDL = p_canmsg->id;
    } else {
        RXB0SIDH = p_canmsg->id >> 3;
        RXB0SIDL = p_canmsg->id << 5;
    }

    RXB0DLC = length;

    if (p_canmsg->flags.rtr) {
        RXB0DLC = length | 0x40;
    } else {
        if (length) {
            unsigned char *pTxBuf, i; 
            
            pTxBuf = &RXB0D0;
            for (i = 0; i < length; i++) {
                *pTxBuf++ = p_canmsg->data[i];
            }
        }
    }
   
    RXB0CON |= 8;

    return 1;
}

/*
 * \brief Read out one can message from controller
 *
 * \param p_canmsg Pointer to can message structure to fill
 * \return 1 on success, 0 if there is no message to read
 */
unsigned char can_receive_message(canmsg_t * p_canmsg) {

    //unsigned char address;    

    if (nFIFOEMPTY == 0) return 0;

    // store timestamp
    p_canmsg->timestamp = clock_getMS();        

    ECANCON &= 0xE0;
    ECANCON |= ((CANCON & 0x07) | 0x10);

    if (RXB0FUL == 0) return 0;

    unsigned char sidh = RXB0SIDH;
    unsigned char sidl = RXB0SIDL;

    if (sidl & 0x08) {
        // extended
        p_canmsg->flags.extended = 1;
        p_canmsg->id  = (unsigned long) sidh << 21;
        p_canmsg->id |= (unsigned long)(sidl & 0xe0) << 13;
        p_canmsg->id |= (unsigned long)(sidl & 0x03) << 16;
        p_canmsg->id |= (unsigned long)(RXB0EIDH)    << 8;
        p_canmsg->id |= (unsigned long) RXB0EIDL;
        unsigned char dlc = RXB0DLC;
        p_canmsg->dlc = dlc & 0x0f;
        p_canmsg->flags.rtr = (dlc >> 6) & 0x01;
    } else {
        // standard
        p_canmsg->flags.extended = 0;
        p_canmsg->flags.rtr = (sidl >> 4) & 0x01;
    	p_canmsg->id  = (unsigned long) sidh << 3;
        p_canmsg->id |= (unsigned long) sidl >> 5;
        p_canmsg->dlc = RXB0DLC & 0x0f;
    }

    // get data
    if (!p_canmsg->flags.rtr) {
        unsigned char i, *ptr;
        unsigned char length = p_canmsg->dlc;
        if (length > 8) length = 8;
        ptr = &RXB0D0;
        for (i = 0; i < length; i++) {
            p_canmsg->data[i] = *ptr++;
        }
    }

    RXB0FUL = 0;
    NOP();
    RXBnIF = 0;
    
    return 1;
}
