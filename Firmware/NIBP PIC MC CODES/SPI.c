/* 
 * File:   ADC.h
 * Author: Naveen Gangadharan
 *
 * Created on 12 August 2020 Wednesday, 00:42
 */


#include "spi.h"
#include <xc.h>


void SPI1_Init(void)
{
    TRISCbits.TRISC3 = 1; 
    TRISCbits.TRISC4 = 1; 
    TRISCbits.TRISC5 = 0; 
    TRISAbits.TRISA5 = 1; 
    ANCON0bits.PCFG4 = 1; 
    SSP1CON1 = 0x04;      
    SSP1STAT = 0;
    SSP1CON1bits.SSPEN = 1; 
    PIR1bits.SSP1IF = 0;  
    PIE1bits.SSP1IE = 1; 
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    return;
}

