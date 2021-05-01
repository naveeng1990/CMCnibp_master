
/* 
 * File:   ADC.h
 * Author: Naveen Gangadharan
 *
 * Created on 12 August 2020 Wednesday, 00:42
 */

#include "I2C.h"
#include <xc.h>

void I2C2_Init(const unsigned long c)
{
    SSP2CON1 = 0x28;
    SSP2CON2 = 0;
    SSP2ADD = (_XTAL_FREQ/(4*c))-1;
    SSP2STAT = 0x80;
    TRISBbits.TRISB4 = 1;  //SCL2
    TRISBbits.TRISB5 = 1;  //SDA2
    return;
}

void I2C2_Wait(void)
{
    while((SSP2STAT&0x04)||(SSP2CON2&0x1F));
    return;
}

void I2C2_Start(void)
{
    I2C2_Wait();
    SSP2CON2bits.SEN = 1;
    return;
}

void I2C2_RepeatedStart(void)
{
    I2C2_Wait();
    SSP2CON2bits.RSEN = 1;
    return;
}

void I2C2_Stop(void)
{
    I2C2_Wait();
    SSP2CON2bits.PEN = 1;
    return;
}

void I2C2_Write(unsigned char b)
{
    I2C2_Wait();
    SSP2BUF = b;
    return;
}

void I2C2_WriteMulti(unsigned char *b, unsigned char l)
{
    unsigned char i;
    for(i=0; i<l; i++)
    {
        I2C2_Wait();
        SSP2BUF = *b;
        b++;
    }
    return;
}