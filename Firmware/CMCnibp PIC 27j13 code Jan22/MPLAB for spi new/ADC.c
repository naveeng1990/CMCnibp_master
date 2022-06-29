/* 
 * File:   ADC.c
 * Author: Naveen Gangadharan
 *
 * Updated on 15 Dec 2021 Wednesday, 00:42
 */

#include "ADC.h"
#include <stdlib.h>
#include<xc.h>

extern float PRS_CALIB;

void ADC_Init()
{
    TRISA |= 0xFF;
    ADCON1 = 0x91;
    ANCON0 &= 0xF0;
    ADCON0 |= 0x01;
}

float ADC_Read(unsigned char channel)
{
    ADCON0 &= 0x03;
    ADCON0 |= channel << 2;
    while(ADCON0bits.GO_DONE);
    ADCON0bits.GO_DONE = 1;
    while(ADCON0bits.GO_DONE);
    return (float)((ADRESH << 8)+ADRESL);
}

unsigned char ADC_BatteryLevel()
{
    float adcConv = (ADC_Read(1)*3.3*2.545)/1024;
    if(adcConv > (float)((BATT_VOLT - BATT_MIN)*3/4 + BATT_MIN)) {return 4;}
    else if(adcConv > (float)((BATT_VOLT - BATT_MIN)*2/4 + BATT_MIN)) {return 3;}
    else if(adcConv > (float)((BATT_VOLT - BATT_MIN)/4 + BATT_MIN)) {return 2;}
    else if(adcConv > BATT_MIN) {return 1;}
    else {return 0;}
}

float ADC_ReadmmHg()
{
    float adcConv = ADC_Read(0);
    adcConv -= 40.96;
    adcConv *= 1000;
    adcConv /= 1024;
    adcConv *= 10;
    adcConv /= 24;
    adcConv -= PRS_CALIB;
    if (adcConv<0) adcConv = 0;
    if (adcConv>380) adcConv = 380;
    return adcConv;
}

void Pressure_Calibrate()
{
    PRS_CALIB = ADC_ReadmmHg();
    return;
}