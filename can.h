#ifndef _CAN_H_INC
#define _CAN_H_INC

// timing for Fosc = 16MHz	 // KVN
#define CAN_TIMINGS_10K  0x00 | 49, 0x80 | ((4-1)<<3) | (7-1), 4-1	// Prop=7, PS1=4, PS2=4, SamplePoint=75%
#define CAN_TIMINGS_20K  0x00 | 39, 0x80 | ((2-1)<<3) | (5-1), 2-1	// Prop=5, PS1=2, PS2=2, SamplePoint=80%
#define CAN_TIMINGS_50K  0x00 | 15, 0x80 | ((2-1)<<3) | (5-1), 2-1	// Prop=5, PS1=2, PS2=2, SamplePoint=80%
#define CAN_TIMINGS_100K 0x00 | 07, 0x80 | ((2-1)<<3) | (5-1), 2-1	// Prop=5, PS1=2, PS2=2, SamplePoint=80%
#define CAN_TIMINGS_125K 0x00 | 07, 0x80 | ((2-1)<<3) | (3-1), 2-1	// Prop=3, PS1=2, PS2=2, SamplePoint=75%
#define CAN_TIMINGS_250K 0x00 | 03, 0x80 | ((2-1)<<3) | (3-1), 2-1	// Prop=3, PS1=2, PS2=2, SamplePoint=75%
#define CAN_TIMINGS_500K 0x00 | 01, 0x80 | ((2-1)<<3) | (3-1), 2-1	// Prop=3, PS1=2, PS2=2, SamplePoint=75%
#define CAN_TIMINGS_800K 0x00 | 00, 0x80 | ((2-1)<<3) | (5-1), 2-1	// Prop=5, PS1=2, PS2=2, SamplePoint=80%
#define CAN_TIMINGS_1M   0x00 | 00, 0x80 | ((2-1)<<3) | (3-1), 2-1	// Prop=3, PS1=2, PS2=2, SamplePoint=75%

// can message data structure
typedef struct
{
    unsigned long id; 			// identifier (11 or 29 bit)
    struct {
       unsigned char rtr : 1;		// remote transmit request
       unsigned char extended : 1;	// extended identifier
    } flags;

    unsigned char dlc;                  // data length code
    unsigned char data[8];		// payload data
    unsigned short timestamp;           // timestamp
} canmsg_t;

// function prototypes
//extern unsigned char can_init();
extern unsigned char can_read_register(unsigned short);
extern void can_write_register(unsigned short, unsigned char);
//extern void can_bit_modify(unsigned char address, unsigned char mask, unsigned char data);
extern void can_set_SJA1000_filter_mask(unsigned char, unsigned char, unsigned char, unsigned char);
extern void can_set_SJA1000_filter_code(unsigned char, unsigned char, unsigned char, unsigned char);
extern unsigned char can_read_errorflags(void);
extern void can_set_bittiming(unsigned char, unsigned char, unsigned char);
extern unsigned char can_send_message(canmsg_t *);
extern unsigned char can_receive_message(canmsg_t *);

#endif
