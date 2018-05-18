/************************************************************************//**
 * \file	picebs.c
 * \brief	PICEBS2 configuration file and includes
 ***************************************************************************/

#include "picebs2.h"

#include <xc.h>
#include <stdio.h>
#include <htc.h>

/**
 * Put a character on the RS232 line, used by printf
 * @param data
 */
void putch(char data) {
    while (!TXIF)
        continue;

    TXREG = data;
}

void enable_high_speed() {
    PLLEN = 1; // activate PLL x4
    OSCCON = 0b01110000; // for 64MHz cpu clock (default is 8MHz)
    // Caution -> the PLL needs up to 2 [ms] to start !
    // Wait a moment if needed with: __delay_ms(2);
}

void configure_uart(void) {
    TRISG0 = 0; // RS232 driver activation
    LATG0 = 1; // for USART communication
    SPBRGH1 = 0x00; // Baud rate = 115200 (standard value)
    SPBRG1 = 16; // for Fosc @ 8MHz
    BRGH1 = 1; // High speed
    BRG161 = 1; // 16 bits baud rate generator
    SYNC1 = 0; // Asynchronous mode
    SPEN1 = 1; // Serial port enabled (configure rx, tx pins)
    CREN1 = 1; // enable receiver
    TXEN1 = 1; // enable transmission
}
