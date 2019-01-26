#ifndef __CONFIG_H_INC
#define __CONFIG_H_INC

#include <xc.h>

#define VERSION_HARDWARE_MAJOR 1
#define VERSION_HARDWARE_MINOR 0
#define VERSION_FIRMWARE_MAJOR 1
#define VERSION_FIRMWARE_MINOR 8

#define BOOTLOADER_ENTRY_ADDRESS 0x7D00

#define STATE_CONFIG 0
#define STATE_OPEN 1
#define STATE_LISTEN 2

#define _XTAL_FREQ  16000000UL
#define BAUDRATE    38400UL
#define BRGVALUE    ((_XTAL_FREQ + 2*BAUDRATE) / (4*BAUDRATE) - 1)

#if	 defined(__DEBUG)
    #define UxSPBRG         SPBRG2
    #define UxSPBRGH        SPBRGH2
    #define UxRCSTA         RCSTA2
    #define UxTXSTA         TXSTA2
    #define UxRCREG         RCREG2
    #define UxTXREG         TXREG2
    #define UxPIR           PIR3
    #define UxRCIF          PIR3bits.RC2IF
    #define UxTXIF          PIR3bits.TX2IF
    #define UxRCIE          PIE3bits.RC2IE
    #define UxTXIE          PIE3bits.TX2IE
    #define UxBAUDCON       BAUDCON2
    #define OERR()          OERR2

    #define stateLED(value) LATA0  = (value)
    #define errorLED(error) LATA5  = (error)
#else
    #define UxSPBRG         SPBRG1
    #define UxSPBRGH        SPBRGH1
    #define UxRCSTA         RCSTA1
    #define UxTXSTA         TXSTA1
    #define UxRCREG         RCREG1
    #define UxTXREG         TXREG1
    #define UxPIR           PIR1
    #define UxRCIF          PIR1bits.RC1IF
    #define UxTXIF          PIR1bits.TX1IF
    #define UxRCIE          PIE1bits.RC1IE
    #define UxTXIE          PIE1bits.TX1IE
    #define UxBAUDCON       BAUDCON1
    #define OERR()          OERR1

    #define stateLED(value) LATB7  = !(value)
    #define errorLED(error) LATB6  = !(error)
#endif

#define RXBnOVFL()          RXBNOVFL
#define RX_OVFL()           LATA2
#define TX_OVFL()           LATA3

#define SET_RX_OVFL()       LATA2  = 1
#define SET_TX_OVFL()       LATA3  = 1
#define RESET_RX_OVFL()     LATA2  = 0
#define RESET_TX_OVFL()     LATA3  = 0

#define cmdSend()           UxTXIE = 1
#define cmdStop()           UxTXIE = 0

#endif
