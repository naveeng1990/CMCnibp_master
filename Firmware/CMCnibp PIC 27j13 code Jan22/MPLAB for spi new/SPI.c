/* 
 * File:   SPI.c
 * Author: Naveen Gangadharan
 *
 * Updated on 12 Dec 2021 Wednesday, 00:42
 */

#include "spi.h"
#include <xc.h>


void SPI1_Init(void)
{
    TRISCbits.TRISC3 = 1; //SCL1 input (for slave mode)
    TRISCbits.TRISC4 = 1; //SDI1 input
    TRISCbits.TRISC5 = 0; //SDO1 output
    TRISAbits.TRISA5 = 1; //#SS pin as input (slave mode)
    ANCON0bits.PCFG4 = 1; //#SS pin as digital input
    SSP1CON1 = 0x04;      //00010100 (spi slave mode, #SS pin enabled)
    SSP1STAT = 0;
    SSP1CON1bits.SSPEN = 1; //Enable MSSP
    PIR1bits.SSP1IF = 0;  //Clear SSP interrupt flag
    PIE1bits.SSP1IE = 1;  //SSP Interrupt Enabled
    INTCONbits.PEIE = 1;
    INTCONbits.GIE = 1;
    return;
}
//
//void SPI_Get(void)
//{
//    unsigned char =value1, value2, temp; 
//    PORTCbits.RC2=0; //cs en
//    SSPBUF=value1; //send hi nib
//    while(!SSPSTATbits.BF);  //wait for sending fin
//    PIR1bits.SSPIF= 0x00;
//    temp=SSPBUF; //clear rcv reg
//    SSPBUF=value2; //send low 6bits
//    while(!SSPSTATbits.BF); //wait for sending fin
//    PIR1bits.SSPIF=0X00;
//    temp=SSPBUF; //clear rcv reg
//    PORTCbits.RC2=1;//cs dis
//    
//}

