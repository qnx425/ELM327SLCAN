#include <xc.h>
#include <stdint.h>
#include "can.h"
#include "clock.h"
#include "elm327slcan.h"
#include "frontend.h"
#include "rbuf.h"

extern uint8_t state;
extern rbuf_t  sRing;

#define cmdPush(data)   do {                                        \
                            if (!rbuf_push((rbuf_t *)&sRing, data)) \
                                SET_RX_OVFL();                       \
                        }   while (0)

unsigned char timestamping = 0;

/**
 * Parse hex value of given string
 *
 * @param line Input string
 * @param len Count of characters to interpret
 * @param value Pointer to variable for the resulting decoded value
 * @return 0 on error, 1 on success
 */
unsigned char parseHex(char * line, unsigned char len, unsigned long * value) {
    *value = 0;
    while (len--) {
        if (*line == 0) return 0;
        *value <<= 4;
        if ((*line >= '0') && (*line <= '9')) {
           *value += *line - '0';
        } else if ((*line >= 'A') && (*line <= 'F')) {
           *value += *line - 'A' + 10;
        } else if ((*line >= 'a') && (*line <= 'f')) {
           *value += *line - 'a' + 10;
        } else return 0;
        line++;
    }
    return 1;
}

/**
 * Send given byte value as hexadecimal string
 *
 * @param value Byte value to send over UART
 */
void sendByteHex(unsigned char value) {

//    sendHex(value, 2);
    
    unsigned char ch = value >> 4;
    if (ch > 9) ch = ch - 10 + 'A';
    else ch = ch + '0';
    cmdPush(ch);

    ch = value & 0xF;
    if (ch > 9) ch = ch - 10 + 'A';
    else ch = ch + '0';
    cmdPush(ch);
    
}

/**
 * Interprets given line and transmit can message
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_transmit(char *line) {
    canmsg_t canmsg;
    unsigned long temp;
    unsigned char idlen;

    canmsg.flags.rtr = ((line[0] == 'r') || (line[0] == 'R'));

    // upper case -> extended identifier
    if (line[0] < 'Z') {
        canmsg.flags.extended = 1;
        idlen = 8;
    } else {
        canmsg.flags.extended = 0;
        idlen = 3;
    }

    if (!parseHex(&line[1], idlen, &temp)) return 0;
    canmsg.id = temp;

    if (!parseHex(&line[1 + idlen], 1, &temp)) return 0;
    canmsg.dlc = temp;

    if (!canmsg.flags.rtr) {
        unsigned char i;
        unsigned char length = canmsg.dlc;
        if (length > 8) length = 8;
        for (i = 0; i < length; i++) {
            if (!parseHex(&line[idlen + 2 + i*2], 2, &temp)) return 0;
            canmsg.data[i] = temp;
        }
    }

    return can_send_message(&canmsg);
}

/**
 * Interprets given line and set up bit timing
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_setupUserdefined(char * line) {
    
    if (state == STATE_CONFIG) {
        unsigned long cnf1, cnf2, cnf3;
        if (parseHex(&line[1], 2, &cnf1) && parseHex(&line[3], 2, &cnf2) && parseHex(&line[5], 2, &cnf3)) {
            can_set_bittiming(cnf1, cnf2, cnf3);
            return CR;
        }
    } 
    return BELL;
}

/**
 * Interprets given line and reads register
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_readRegister(char * line) {
    
    unsigned long address;
    if (parseHex(&line[1], 3, &address)) {
        unsigned char value = can_read_register(address);
        sendByteHex(value);
        return CR;
    }
    return BELL;
}

/**
 * Interprets given line and writes register
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_writeRegister(char * line) {
        
    unsigned long address, data;
    if (parseHex(&line[1], 3, &address) && parseHex(&line[4], 2, &data)) {
        can_write_register(address, data);
        return CR;
    }
    return BELL;
}

/**
 * Interprets given line and set time stamping
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_setTimestamping(char * line) {
    
    unsigned long stamping;
    if (parseHex(&line[1], 1, &stamping)) {
        timestamping = (stamping != 0);
        return CR;
    }
    
    return BELL;
}

/**
 * Send out given error flags
 * 
 * @param flags Error flags to send out
 */
void frontend_sendErrorflags(unsigned char flags) {
    
    cmdPush('F');
    sendByteHex(flags);
    cmdPush(CR);
}

/**
 * Interprets given line and handle status flag requests
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_errorFlags(char * line) {
    unsigned char flags = COMSTAT & 0x7F;

    cmdPush('F');
    sendByteHex(flags);
    return CR;
}

/**
 * Interprets given line and handle error reporting requests
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_errorReporting(char * line) {

    unsigned long subcmd = 0;
    
    if (parseHex(&line[1], 1, &subcmd)) {
        if (subcmd == 2) {
            RXBNOVFL = 0;
            RESET_RX_OVFL();
            RESET_TX_OVFL();
            errorLED(0);
            return CR;  
        }
    }
    
    return BELL;    
}

/**
 * Interprets given line and set filter mask
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_setFilterMask(char * line) {
    if (state == STATE_CONFIG)
    {
        unsigned long am0, am1, am2, am3;
        if (parseHex(&line[1], 2, &am0) && parseHex(&line[3], 2, &am1) && parseHex(&line[5], 2, &am2) && parseHex(&line[7], 2, &am3)) {
            can_set_SJA1000_filter_mask(am0, am1, am2, am3);
            return CR;
        }
    } 
    
    return BELL;
}

/**
 * Interprets given line and set filter code
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_setFilterCode(char * line) {
    if (state == STATE_CONFIG)
    {
        unsigned long ac0, ac1, ac2, ac3;
        if (parseHex(&line[1], 2, &ac0) && parseHex(&line[3], 2, &ac1) && parseHex(&line[5], 2, &ac2) && parseHex(&line[7], 2, &ac3)) {
            can_set_SJA1000_filter_code(ac0, ac1, ac2, ac3);
            return CR;
        }
    } 
    return BELL;
}

/**
 * Interprets given line and jump to bootloader
 *
 * @param line Line string which contains the transmit command
 */
unsigned char parseCmd_bootloaderJump(char * line) {
    
    unsigned long magic;
    if (parseHex(&line[1], 2, &magic)) {
                    
        // check magic code and if bootloader version is new enough to supports bootloader entry via jump
        if (magic == 0x10) {
            CANCON = 0x80;
            INTCON = 0;
            #asm
                goto BOOTLOADER_ENTRY_ADDRESS
            #endasm
        }             
    }
            
    return BELL;
}


/**
 * Parse given command line
 *
 * @param line Line string to parse
 */
void parseLine(char * line) {

    unsigned char result = BELL;
    
    switch (line[0]) {
        case 'S': // Setup with standard CAN bitrates
            if (state == STATE_CONFIG)
            {
                switch (line[1]) {
                    case '0': can_set_bittiming(CAN_TIMINGS_10K);  result = CR; break;
                    case '1': can_set_bittiming(CAN_TIMINGS_20K);  result = CR; break;
                    case '2': can_set_bittiming(CAN_TIMINGS_50K);  result = CR; break;
                    case '3': can_set_bittiming(CAN_TIMINGS_100K); result = CR; break;
                    case '4': can_set_bittiming(CAN_TIMINGS_125K); result = CR; break;
                    case '5': can_set_bittiming(CAN_TIMINGS_250K); result = CR; break;
                    case '6': can_set_bittiming(CAN_TIMINGS_500K); result = CR; break;
                    case '7': can_set_bittiming(CAN_TIMINGS_800K); result = CR; break;
                    case '8': can_set_bittiming(CAN_TIMINGS_1M);   result = CR; break;
                }

            }
            break;
        case 's': // Setup with user defined timing settings for CNF1/CNF2/CNF3
            result = parseCmd_setupUserdefined(line);
            break;

        case 'G': // Read given register
            result = parseCmd_readRegister(line);
            break;

        case 'W': // Write given register
            result = parseCmd_writeRegister(line);
            break;

        case 'V': // Get hardware version
            {

                cmdPush('V');
                sendByteHex(VERSION_HARDWARE_MAJOR);
                sendByteHex(VERSION_HARDWARE_MINOR);
                result = CR;
            }
            break;
        case 'v': // Get firmware version
            {
                
                cmdPush('v');
                sendByteHex(VERSION_FIRMWARE_MAJOR);
                sendByteHex(VERSION_FIRMWARE_MINOR);
                result = CR;
            }
            break;
        case 'N': // Get serial number
            {
                cmdPush('N');
                cmdPush('1');
                cmdPush('9');
                cmdPush('7');
                cmdPush('6');
                result = CR;
            }
            break;     
        case 'O': // Open CAN channel
            if (state == STATE_CONFIG) {
                // normal mode
                CANCON = 0x00; 
                while ((CANSTAT & 0xE0) != 0x00) continue; 

                clock_reset();

                state = STATE_OPEN;
                result = CR;
            }
            break;
            
        case 'l': // Loop-back mode
            if (state == STATE_CONFIG) {
                CANCON = 0x40; 
                while ((CANSTAT & 0xE0) != 0x40) continue; 

                state = STATE_OPEN;
                result = CR;
            }
            break; 
            
        case 'L': // Open CAN channel in listen-only mode
            if (state == STATE_CONFIG) {
                CANCON = 0x60; 
                while ((CANSTAT & 0xE0) != 0x60) continue; 

                state = STATE_LISTEN;
                result = CR;
            }
            break; 
            
        case 'C': // Close CAN channel
            if (state != STATE_CONFIG) {
                CANCON = 0x80; 
                while ((CANSTAT & 0xE0) != 0x80) continue; 

                state = STATE_CONFIG;
                result = CR;
            }
            break; 
            
        case 'r': // Transmit standard RTR (11 bit) frame
        case 'R': // Transmit extended RTR (29 bit) frame
        case 't': // Transmit standard (11 bit) frame
        case 'T': // Transmit extended (29 bit) frame
            if (state == STATE_OPEN) {
                if (parseCmd_transmit(line)) {
                    if (line[0] < 'Z') cmdPush('Z');
                    else cmdPush('z');
                    result = CR;
                }
            }
            break;        

        case 'f': // Handle error reporting requests
            result = parseCmd_errorReporting(line);
            break;

        case 'F': // Handle status flag requests
            result = parseCmd_errorFlags(line);
            break;

        case 'Z': // Set time stamping
            result = parseCmd_setTimestamping(line);
            break;

        case 'm': // Set accpetance filter mask
            result = parseCmd_setFilterMask(line);
            break;

        case 'M': // Set accpetance filter code
            result = parseCmd_setFilterCode(line);
            break;
 
        case 'B': // Jump to bootloader
            result = parseCmd_bootloaderJump(line);
            break;
    }

   cmdPush(result);
   
   cmdSend();
}

uint8_t canmsg2ascii(canmsg_t * canmsg, char * s) {
    char     ch, step = RX_STEP_TYPE;    
    uint8_t  num = 0;
    
    do {
        if (step == RX_STEP_TYPE) {
            // type
            if (canmsg->flags.extended) {                 
                step = RX_STEP_ID_EXT;                
                if (canmsg->flags.rtr) *s++ = 'R';
                else *s++ = 'T';
                num++;
            } else {
                step = RX_STEP_ID_STD;
                if (canmsg->flags.rtr) *s++ = 'r';
                else *s++ = 't';
                num++;
            }
        } else if (step < RX_STEP_DLC) {
            // id        
            unsigned char i = step - 1;
            unsigned char * id_bp = (unsigned char*) &canmsg->id;
        
            ch = id_bp[3 - (i / 2)];
            if ((i % 2) == 0) ch = ch >> 4;
            
            ch = ch & 0xF;
            if (ch > 9) ch = ch - 10 + 'A';
            else ch = ch + '0';
            
            *s++ = ch;
            num++;
            
            step++;
        } else if (step < RX_STEP_DATA) {
            // length        
            ch = canmsg->dlc;
            
            ch = ch & 0xF;
            if (ch > 9) ch = ch - 10 + 'A';
            else ch = ch + '0';

            *s++ = ch;
            num++;
            
            if ((canmsg->dlc == 0) || canmsg->flags.rtr) step = RX_STEP_TIMESTAMP;
            else step++;
        } else if (step < RX_STEP_TIMESTAMP) {
            // data        
            unsigned char i = step - RX_STEP_DATA;
        
            ch = canmsg->data[i / 2];
            if ((i % 2) == 0) ch = ch >> 4;
            
            ch = ch & 0xF;
            if (ch > 9) ch = ch - 10 + 'A';
            else ch = ch + '0';

            *s++ = ch;
            num++;
            
            step++;        
            if (step - RX_STEP_DATA == canmsg->dlc*2) step = RX_STEP_TIMESTAMP;
        } else if (timestamping && (step < RX_STEP_CR)) {
            // timestamp
            unsigned char i = step - RX_STEP_TIMESTAMP;

            if (i < 2) ch = (canmsg->timestamp >> 8) & 0xff;
            else ch = canmsg->timestamp & 0xff;
            if ((i % 2) == 0) ch = ch >> 4;
            
            ch = ch & 0xF;
            if (ch > 9) ch = ch - 10 + 'A';
            else ch = ch + '0';
            
            *s++ = ch;
            num++;
            
            step++;        
        } else {
            // linebreak
            *s = CR;
            num++;
            
            step = RX_STEP_FINISHED;
        }
    }   while (step != RX_STEP_FINISHED);

    return num;
}
