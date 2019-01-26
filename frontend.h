#ifndef _FRONTEND_
#define _FRONTEND_

#define LINE_MAXLEN 100
#define BELL 7
#define CR 13
#define LR 10

#define RX_STEP_TYPE 0
#define RX_STEP_ID_EXT 1
#define RX_STEP_ID_STD 6
#define RX_STEP_DLC 9
#define RX_STEP_DATA 10
#define RX_STEP_TIMESTAMP 26
#define RX_STEP_CR 30
#define RX_STEP_FINISHED 0xff

extern unsigned char transmitStd(char *);
extern void parseLine(char *);
extern char canmsg2ascii_getNextChar(canmsg_t *, unsigned char *);
extern void sendbuffer_send(void);
extern unsigned char sendbuffer_isEmpty(void);
extern void sendStatusflags(unsigned char);
extern void frontend_sendErrorflags(unsigned char);

extern unsigned char canmsg2ascii(canmsg_t *, char *);

#endif
