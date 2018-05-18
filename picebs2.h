/************************************************************************//**
 * \file	picebs.h
 * \brief	PICEBS2 configuration file and includes
 * \author P.A. Mudry
 * Version: 1.11
 * Release notes:
 *  1.00 Initial release 
 *  1.10 Removed PLIB dependency, set value for UART
 *  1.11 Split .c and .h
 ***************************************************************************/

#ifndef PICEBS2_H
#define	PICEBS2_H

#include <stdio.h>
#include <xc.h>

/*****************************************************************************
 * Processor configuration
 * \see PIC18F87K22 §26.1 Configuration bits
 ****************************************************************************/
#pragma config RETEN = OFF		// Vreg sleep
#pragma config INTOSCSEL = LOW	// INTOSC is low power mode
#pragma config SOSCSEL = LOW	// Secondary oscillator mode
#pragma config XINST = OFF		// no extended instruction mode
#pragma config FOSC = INTIO2	// internal oscillator 16MHz
#pragma config PLLCFG = ON		// PLL on, runs at 64MHz
#pragma config FCMEN = OFF		// fail safe clock disable
#pragma config IESO = OFF		// two speed start clock disable
#pragma config PWRTEN = ON		// power up timer is turned on
#pragma config BOREN = OFF		// brown out detection is activated
#pragma config BORV = 2			// brown out voltage is selected as 2.0V
#pragma config BORPWR = LOW		// brown out detection is low power
#pragma config WDTEN = OFF		// watchdog timer is turned off
#pragma config WDTPS = 1		// watchdog postscaler 1:1 (not used
#pragma config RTCOSC = SOSCREF	// RTC uses the secondary oscillator
#pragma config CCP2MX = PORTBE	// CCP2MX is RE7 (why not, not used
#pragma config MSSPMSK = MSK7	// MSSP slave address mode not used
#pragma config MCLRE = ON		// reset pin is dedicated as reset function
#pragma config STVREN = ON		// stack overflow create a reset
#pragma config BBSIZ = BB1K		// boot block (not used
#pragma config DEBUG = ON		// pin PGC PGD are dedicated for debug

#pragma config CP0 = OFF		// no protections are activated
#pragma config CP1 = OFF
#pragma config CP2 = OFF
#pragma config CP3 = OFF
#pragma config CP4 = OFF
#pragma config CP5 = OFF
#pragma config CP6 = OFF
#pragma config CP7 = OFF
#pragma config CPB = OFF
#pragma config CPD = OFF
#pragma config WRT0 = OFF
#pragma config WRT1 = OFF
#pragma config WRT2 = OFF
#pragma config WRT3 = OFF
#pragma config WRT4 = OFF
#pragma config WRT5 = OFF
#pragma config WRT6 = OFF
#pragma config WRT7 = OFF
#pragma config WRTB = OFF
#pragma config WRTC = OFF
#pragma config WRTD = OFF
#pragma config EBRT0 = OFF
#pragma config EBRT1 = OFF
#pragma config EBRT2 = OFF
#pragma config EBRT3 = OFF
#pragma config EBRT4 = OFF
#pragma config EBRT5 = OFF
#pragma config EBRT6 = OFF
#pragma config EBRT7 = OFF
#pragma config EBRTB = OFF

// Our F_clk is running at 64 MHz as we are using high speed
#define _XTAL_FREQ 64000000L

/**
 * Put a character on the RS232 line, used by printf
 * @param data
 */
void putch(char data);

/**
 * Activates high speed PLL to have a clock@64 MHz
 * which enables a F_Instruction of 16 MHz
 */
void enable_high_speed();

/**
 * Configures the UART to 115200, no parity, 8 bits
 */
void configure_uart(void);

#endif /* PICEBS2_H */
