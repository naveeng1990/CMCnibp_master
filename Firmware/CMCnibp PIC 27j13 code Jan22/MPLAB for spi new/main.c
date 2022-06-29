/* 
 * File:   main.c
 * Author: Naveen Gangadharan
 *
 * Updated on 15 Dec 2021 Wednesday, 00:42
 */

#pragma config WDTEN = OFF      // Watchdog Timer (Disabled - Controlled by SWDTEN bit)
#pragma config PLLDIV = 1       // 96MHz PLL Prescaler Selection (PLLSEL=0) (No prescale (4 MHz oscillator input drives PLL directly))
#pragma config CFGPLLEN = OFF   // PLL Enable Configuration Bit (PLL Disabled)
#pragma config STVREN = ON      // Stack Overflow/Underflow Reset (Enabled)
#pragma config XINST = OFF      // Extended Instruction Set (Disabled)
// CONFIG1H
#pragma config CP0 = OFF        // Code Protect (Program memory is not code-protected)
// CONFIG2L
#pragma config OSC = INTOSC     // Oscillator (INTOSC)
#pragma config SOSCSEL = HIGH   // T1OSC/SOSC Power Selection Bits (High Power T1OSC/SOSC circuit selected)
#pragma config CLKOEC = OFF     // EC Clock Out Enable Bit  (CLKO output disabled on the RA6 pin)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor (Enabled)
#pragma config IESO = OFF       // Internal External Oscillator Switch Over Mode (Disabled)
// CONFIG2H
#pragma config WDTPS = 32768    // Watchdog Postscaler (1:32768)
// CONFIG3L
#pragma config DSWDTOSC = INTOSCREF// DSWDT Clock Select (DSWDT uses INTRC)
#pragma config RTCOSC = INTOSCREF// RTCC Clock Select (RTCC uses INTRC)
#pragma config DSBOREN = OFF    // Deep Sleep BOR (Disabled)
#pragma config DSWDTEN = OFF    // Deep Sleep Watchdog Timer (Disabled)
#pragma config DSWDTPS = G2     // Deep Sleep Watchdog Postscaler (1:2,147,483,648 (25.7 days))
// CONFIG3H
#pragma config IOL1WAY = OFF    // IOLOCK One-Way Set Enable bit (The IOLOCK bit (PPSCON<0>) can be set and cleared as needed)
#pragma config ADCSEL = BIT10   // ADC 10 or 12 Bit Select (10 - Bit ADC Enabled)
#pragma config PLLSEL = PLL4X   // PLL Selection Bit (Selects 4x PLL)
#pragma config MSSP7B_EN = MSK7 // MSSP address masking (7 Bit address masking mode)
// CONFIG4L
#pragma config WPFP = PAGE_127  // Write/Erase Protect Page Start/End Location (Write Protect Program Flash Page 127)
#pragma config WPCFG = OFF      // Write/Erase Protect Configuration Region  (Configuration Words page not erase/write-protected)
// CONFIG4H
#pragma config WPDIS = OFF      // Write Protect Disable bit (WPFP<6:0>/WPEND region ignored)
#pragma config WPEND = PAGE_WPFP// Write/Erase Protect Region Select bit (valid when WPDIS = 0) (Pages WPFP<6:0> through Configuration Words erase/write protected)

#define _XTAL_FREQ  8000000

#include <xc.h> 
#include <stdint.h>
#include "UI.h"
#include "OLED.h"
#include "SSD1306.h"
#include "I2C.h"
#include "ADC.h"
#include "SPI.h"

#define SWA         PORTBbits.RB0
#define SWB         PORTBbits.RB1
#define SWC         PORTBbits.RB2
#define LED1        PORTBbits.RB6
#define LED2        PORTBbits.RB7
#define VALVE1      PORTCbits.RC0
#define VALVE2      PORTCbits.RC1
#define PUMP1       PORTCbits.RC2
//#define PUMP2       PORTCbits.RC3
#define PUMP2       PORTBbits.RB3
#define LEVEL_DEF   0
#define LEVEL_LOAD  1

#define GAP_SMOOTH  0//5//10 on device 02- supply powered one

extern uint8_t m_font;

unsigned char lastBattery = 0;
unsigned char menu = 0;
unsigned char uRec = 0;
unsigned char act = 0;
float PRS_CALIB = 0;
float loadedLevel = 0;
int pressureLevel[16];
unsigned char uL=0;

void setPressureLevels(uint8_t level);
void checkBatt();
void errorBatt();
void deviceRun();
void rampRun();
void softStart();
void softStop();

void __interrupt() SPI1_INT(void)
{
    uint8_t c=SSP1BUF;
    PIR1bits.SSP1IF = 0;
    if(act==0)
    {
        if(c>3){act=0;return;}
        act=c;
        if(act==1){act=0;uRec=1;INTCONbits.GIE = 0;}
    }
    else if(act==2)
    {
        loadedLevel=(float)c;
        setPressureLevels(LEVEL_LOAD);
        uRec=2;act=0;
        INTCONbits.GIE = 0;
    }
    else if(act==3)
    {
        pressureLevel[uL]=(int)c;
        uL++;
        if(uL==16){uRec=3;act=0;INTCONbits.GIE = 0;}
        
    }
    else act=0;
    return;
}

void main(void)
{
    OSCCON = 0x70;
    PORTA = 0x00;
    PORTB = 0x00;
    PORTC = 0x00;
    LATA = 0x00;
    LATB = 0x00;
    LATC = 0x00;
//    TRISB = 0x0F;
    TRISB = 0x07;
//    PORTBbits.RB3=0;
    TRISC = 0x00;
    
    
    ANCON0 = 0xFF;
    ANCON1 = 0x1F;
    
    menu = 0;
    SSD1306_Begin(SSD1306_SWITCHCAPVCC);
    ADC_Init();
    SPI1_Init();
    PUMP1 = 0;
    PUMP2 = 0;
    VALVE1 = 1;
    VALVE2 = 1;
    
    lastBattery = ADC_BatteryLevel();
    if(lastBattery == 0) { errorBatt(); }
    __delay_ms(1000);
    Pressure_Calibrate();
    
    while(1)
    {
        loop:
        INTCONbits.GIE = 1;
        checkBatt();
        if(menu == 0)
        {
            UI_Clear();
            OLED_BatteryLevel(lastBattery);
            if(loadedLevel == 0){UI_Ready(LEVEL_DEF);}
            else{UI_Ready(loadedLevel);}
            menu = 1;
        }
        __delay_ms(100);
        checkBatt();
        if(SWA == 0 || uRec == 2 || uRec == 3)
        {
            INTCONbits.GIE = 0;
            
            __delay_ms(500);
//            if(loadedLevel == 0)setPressureLevels(LEVEL_DEF);
//            else setPressureLevels(LEVEL_LOAD);
            UI_Clear();
            OLED_BatteryLevel(lastBattery);
            UI_Proceed(pressureLevel);
            while(SWA != 0 && uRec==0){checkBatt();if(SWB == 0){menu = 0;goto loop;}}
            deviceRun();
            while(SWB == 0);
            uRec = 0;
            menu = 0;
        }
        else if(SWB == 0 || uRec == 1)
        {
            INTCONbits.GIE = 0;
            __delay_ms(500);
            setPressureLevels(LEVEL_DEF);
            UI_Clear();
            OLED_BatteryLevel(lastBattery);
            UI_RampProceed();
            while(SWA != 0 && uRec==0){checkBatt();if(SWB == 0){menu = 0;goto loop;}}
            rampRun();
            while(SWB == 0);
            uRec = 0;
            menu = 0;
        }
        __delay_ms(250);
    }
}

void setPressureLevels(uint8_t level)
{
    if(level == LEVEL_DEF)
    {
       pressureLevel[15] = 30;
        pressureLevel[14] = 40;
        pressureLevel[13] = 50;
        pressureLevel[12] = 60;
        pressureLevel[11] = 70;
        pressureLevel[10] = 80;
        pressureLevel[9] = 90;
        pressureLevel[8] = 100;
        pressureLevel[7] = 110;
        pressureLevel[6] = 120;
        pressureLevel[5] = 130;
        pressureLevel[4] = 140;
        pressureLevel[3] = 150;
        pressureLevel[2] = 160;
        pressureLevel[1] = 170;
        pressureLevel[0] = 180;
    }
    else if(level == LEVEL_LOAD)
    {
        pressureLevel[15] = 30;
        pressureLevel[14] = (int)(30 + ((loadedLevel-30)/11));
        pressureLevel[13] = (int)(30 + (2*((loadedLevel-30)/11)));
        pressureLevel[12] = (int)(30 + (3*((loadedLevel-30)/11)));
        pressureLevel[11] = (int)(30 + (4*((loadedLevel-30)/11)));
        pressureLevel[10] = (int)(30 + (5*((loadedLevel-30)/11)));
        pressureLevel[9] = (int)(30 + (6*((loadedLevel-30)/11)));
        pressureLevel[8] = (int)(30 + (7*((loadedLevel-30)/11)));
        pressureLevel[7] = (int)(30 + (8*((loadedLevel-30)/11)));
        pressureLevel[6] = (int)(30 + (9*((loadedLevel-30)/11)));
        pressureLevel[5] = (int)(30 + (10*((loadedLevel-30)/11)));
        pressureLevel[4] = loadedLevel;
        pressureLevel[3] = loadedLevel+5;
        pressureLevel[2] = loadedLevel+10;
        pressureLevel[1] = loadedLevel+20;
        pressureLevel[0] = loadedLevel+30;
    }
}

void checkBatt()
{
    unsigned char tmp;
    tmp = ADC_BatteryLevel();
    if (tmp != lastBattery)
    {
        lastBattery = tmp;
        if(lastBattery == 0){ errorBatt(); }
        OLED_BatteryLevel(lastBattery);
    }
}

void errorBatt()
{
    UI_Clear();
    UI_LowBattery();
    __delay_ms(1000);
    while(1)
    {
        OLED_ClearLine(0);
        __delay_ms(1000);
        OLED_BatteryLevel(0);
        __delay_ms(1000);
    }
}

void deviceRun()
{
    int i;
    UI_Clear();
    checkBatt();
    OLED_BatteryLevel(lastBattery);
    int order = 0;
    float pressureVal = 0;
    for(order=0;order<16;order++)
    {
        UI_SetPress(pressureLevel[order]);
        VALVE1 = 0;
        VALVE2 = 0;
//        softStart();
        PUMP1 = 1;
        PUMP2 = 1;
        pressureVal = ADC_ReadmmHg();
        while(pressureVal < pressureLevel[order]-GAP_SMOOTH)
        {   
            PUMP1 = 1;
            PUMP2 = 1;
            pressureVal = ADC_ReadmmHg();
            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
            m_font = FONT_SIZE_MEDIUM;
            OLED_ClearLine(2);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
            OLED_ClearLine(3);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
            OLED_ClearLine(4);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
            OLED_ClearLine(5);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
            OLED_SetCursor(50,3);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
            OLED_PrintLong(pressureVal,1);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
            if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
        }
//        softStop();
        PUMP1 = 0;
        PUMP2 = 0;  
        UI_ReadPress(pressureVal);
        for(i=0;i<80;i++)  //35//65
        {
            if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
            VALVE1 = 0;
            VALVE2 = 0;
            pressureVal = ADC_ReadmmHg();
            if(pressureVal < pressureLevel[order]) {PUMP1=1;__delay_ms(5);PUMP1=0;}
            else {PUMP1=0;PUMP2=0;}
            OLED_ClearLine(2);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal < pressureLevel[order]) {PUMP1=1;__delay_ms(5);PUMP1=0;}
            else {PUMP1=0;PUMP2=0;}
            OLED_ClearLine(3);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal < pressureLevel[order]) {PUMP1=1;__delay_ms(5);PUMP1=0;}
            else {PUMP1=0;PUMP2=0;}
            OLED_ClearLine(4);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal < pressureLevel[order]) {PUMP1=1;__delay_ms(5);PUMP1=0;}
            else {PUMP1=0;PUMP2=0;}
            OLED_ClearLine(5);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal < pressureLevel[order]) {PUMP1=1;__delay_ms(5);PUMP1=0;}
            else {PUMP1=0;PUMP2=0;}
            OLED_SetCursor(50,3);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal < pressureLevel[order]) {PUMP1=1;__delay_ms(5);PUMP1=0;}
            else {PUMP1=0;PUMP2=0;}
            OLED_PrintLong(pressureVal,1);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal < pressureLevel[order]) {PUMP1=1;__delay_ms(5);PUMP1=0;}
            else {PUMP1=0;PUMP2=0;}
            __delay_ms(10);
        }
        PUMP1 = 0;
        PUMP2 = 0;
        VALVE1 = 1;
        VALVE2 = 1;
        pressureVal = ADC_ReadmmHg();
        while(pressureVal > 0) //changed from 100
        {
            pressureVal = ADC_ReadmmHg();
            if(pressureVal <= 0) break;
            m_font = FONT_SIZE_MEDIUM;
            OLED_ClearLine(2);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal <= 0) break;
            OLED_ClearLine(3);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal <= 0) break;
            OLED_ClearLine(4);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal <= 0) break;
            OLED_ClearLine(5);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal <= 0) break;
            OLED_SetCursor(50,3);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal <= 0) break;
            OLED_PrintLong(pressureVal,1);
            pressureVal = ADC_ReadmmHg();
            if(pressureVal <= 0) break;
            if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
        }
        m_font = FONT_SIZE_MEDIUM;
        OLED_ClearLine(2);
        OLED_ClearLine(3);
        OLED_ClearLine(4);
        OLED_SetCursor(50,3);
        OLED_PrintLong(pressureVal,1);
        UI_ReadPress(pressureVal);
        for(i=0;i<60;i++) //8
        {
            if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
            __delay_ms(100);
        } //Activate this loop if you'd like the device to wait 10 seconds for next pumping step
    }
    return;
}

//void rampRun()
//{
//    
//    pressureLevel[0] = 30;
//    pressureLevel[1] = 40;
//    pressureLevel[2] = 50;
//    pressureLevel[3] = 60;
//    pressureLevel[4] = 70;
//    pressureLevel[5] = 80;
//    pressureLevel[6] = 90;
//    pressureLevel[7] = 100;
//    pressureLevel[8] = 110;
//    pressureLevel[9] = 120;
//    pressureLevel[10] = 130;
//    pressureLevel[11] = 140;
//    pressureLevel[12] = 150;
//    pressureLevel[13] = 160;
//    pressureLevel[14] = 175;
//    pressureLevel[15] = 180;
//    int i;
//    UI_Clear();
//    checkBatt();
//    OLED_BatteryLevel(lastBattery);
//    int order = 0;
//    float pressureVal = 0;
//    for(order=0;order<16;order++)
//    {
//        UI_SetPress(pressureLevel[order]);
//        VALVE1 = 0;
//        VALVE2 = 0;
////        softStart();
//        PUMP1 = 1;
////        PUMP2 = 1;
//        pressureVal = ADC_ReadmmHg();
//        while(pressureVal < pressureLevel[order]-GAP_SMOOTH)
//        {   
//            PUMP1 = 1;
////            PUMP2 = 1;
//            pressureVal = ADC_ReadmmHg();
//            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
//            m_font = FONT_SIZE_MEDIUM;
//            OLED_ClearLine(2);
//            pressureVal = ADC_ReadmmHg();
//            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
//            OLED_ClearLine(3);
//            pressureVal = ADC_ReadmmHg();
//            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
//            OLED_ClearLine(4);
//            pressureVal = ADC_ReadmmHg();
//            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
//            OLED_ClearLine(5);
//            pressureVal = ADC_ReadmmHg();
//            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
//            OLED_SetCursor(50,3);
//            pressureVal = ADC_ReadmmHg();
//            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
//            OLED_PrintLong(pressureVal,1);
//            pressureVal = ADC_ReadmmHg();
//            if(pressureVal >= pressureLevel[order]-GAP_SMOOTH) break;
//            if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
//        }
//        while(order==15){
//            PUMP1=0;
//            PUMP2=0;
//            VALVE1=1;
//            VALVE2=1;
//            order=180;
//        }
//        __delay_ms(300);
////        softStop();
////        PUMP1 = 0;
////        PUMP2 = 0;  
//        UI_ReadPress(pressureVal);
////        for(i=0;i<65;i++)  //35
////        {
////            if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal < pressureLevel[order]) {PUMP1=1;PUMP2=1;__delay_ms(5);PUMP1=0;PUMP2=0;}
////            else {PUMP1=0;PUMP2=0;}
////            OLED_ClearLine(2);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal < pressureLevel[order]) {PUMP1=1;PUMP2=1;__delay_ms(5);PUMP1=0;PUMP2=0;}
////            else {PUMP1=0;PUMP2=0;}
////            OLED_ClearLine(3);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal < pressureLevel[order]) {PUMP1=1;PUMP2=1;__delay_ms(5);PUMP1=0;PUMP2=0;}
////            else {PUMP1=0;PUMP2=0;}
////            OLED_ClearLine(4);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal < pressureLevel[order]) {PUMP1=1;PUMP2=1;__delay_ms(5);PUMP1=0;PUMP2=0;}
////            else {PUMP1=0;PUMP2=0;}
////            OLED_ClearLine(5);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal < pressureLevel[order]) {PUMP1=1;PUMP2=1;__delay_ms(5);PUMP1=0;PUMP2=0;}
////            else {PUMP1=0;PUMP2=0;}
////            OLED_SetCursor(50,3);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal < pressureLevel[order]) {PUMP1=1;PUMP2=1;__delay_ms(5);PUMP1=0;PUMP2=0;}
////            else {PUMP1=0;PUMP2=0;}
////            OLED_PrintLong(pressureVal,1);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal < pressureLevel[order]) {PUMP1=1;PUMP2=1;__delay_ms(5);PUMP1=0;PUMP2=0;}
////            else {PUMP1=0;PUMP2=0;}
////            __delay_ms(10);
////        }
////        PUMP1 = 0;
////        PUMP2 = 0;
////        VALVE1 = 1;
////        VALVE2 = 1;
////        pressureVal = ADC_ReadmmHg();
////        while(pressureVal > 0) //changed from 100
////        {
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal <= 0) break;
////            m_font = FONT_SIZE_MEDIUM;
////            OLED_ClearLine(2);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal <= 0) break;
////            OLED_ClearLine(3);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal <= 0) break;
////            OLED_ClearLine(4);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal <= 0) break;
////            OLED_ClearLine(5);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal <= 0) break;
////            OLED_SetCursor(50,3);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal <= 0) break;
////            OLED_PrintLong(pressureVal,1);
////            pressureVal = ADC_ReadmmHg();
////            if(pressureVal <= 0) break;
////            if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
////        }
////        m_font = FONT_SIZE_MEDIUM;
////        OLED_ClearLine(2);
////        OLED_ClearLine(3);
////        OLED_ClearLine(4);
////        OLED_SetCursor(50,3);
////        OLED_PrintLong(pressureVal,1);
////        UI_ReadPress(pressureVal);
////        for(i=0;i<70;i++) //8
////        {
////            if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
////            __delay_ms(100);
////        } //Activate this loop if you'd like the device to wait 10 seconds for next pumping step
//        
//  
//    }
//    return;
//}

//
//void rampRun()
//{
//    UI_Clear();
//    checkBatt();
//    OLED_BatteryLevel(lastBattery);
//    float pressureVal = 0;
//    
////    UI_SetPress(pressureLevel[0]);
//    UI_SetPress(190);
//    VALVE1 = 0;
//    VALVE2 = 0;
//    PUMP1 = 1;
//    PUMP2 = 1;
//    pressureVal = ADC_ReadmmHg();
////    while(pressureVal < pressureLevel[0])
//    while(pressureVal < 200)
//    {
//        __delay_ms(15);//20,9,9
//        PUMP1 = 1;
////        PUMP2 = 1;
//        __delay_ms(1);//9,9,15
//        PUMP1 = 0;
//
//        pressureVal = ADC_ReadmmHg();
//        UI_ReadPress(pressureVal);
//        if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
//    }
//    UI_ReadPress(pressureVal);
//    PUMP1 = 0;
//    PUMP2 = 0;
//    VALVE1 = 1;
//    VALVE2 = 1;
//    while(pressureVal > 0)
//    {
//        pressureVal = ADC_ReadmmHg();
//        UI_ReadPress(pressureVal);
//        if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
//    }
//    return;
//}



void rampRun()
{
    UI_Clear();
    checkBatt();
    OLED_BatteryLevel(lastBattery);
    float pressureVal = 0;
    
    UI_SetPress(190);
    VALVE1 = 0;
    VALVE2 = 0;
    PUMP1 = 1;
//    PUMP2 = 1;
    pressureVal = ADC_ReadmmHg();
    while(pressureVal < 190)
    {
        __delay_ms(1);
        PUMP1 = 1;
//        PUMP2 = 1;
        __delay_ms(20);
        PUMP1 = 0;
//        PUMP2 = 0;
        pressureVal = ADC_ReadmmHg();
        UI_ReadPress(pressureVal);
        if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
    }
    UI_ReadPress(pressureVal);
    PUMP1 = 0;
//    PUMP2 = 0;
    VALVE1 = 1;
    VALVE2 = 1;
    while(pressureVal > 0)
    {
        pressureVal = ADC_ReadmmHg();
        UI_ReadPress(pressureVal);
        if(SWB == 0) {VALVE1 = 1;VALVE2 = 1;PUMP1 = 0;PUMP2 = 0;return;}
    }
    return;
}

void softStart()
{
    uint8_t st;
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(1);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(3);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(5);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(7);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(9);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(11);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(13);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(15);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(17);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(19);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(25);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    for(st=0;st<5;st++){PUMP1 = 1;PUMP2 = 1;__delay_ms(35);PUMP1 = 0;PUMP2 = 0;__delay_ms(10);}
    PUMP1 = 1;PUMP2 = 1;
    return;
}

void softStop()
{
    PUMP1 = 1;PUMP2 = 1;__delay_ms(9);
    PUMP1 = 0;PUMP2 = 0;__delay_ms(10);
    PUMP1 = 1;PUMP2 = 1;__delay_ms(8);
    PUMP1 = 0;PUMP2 = 0;__delay_ms(10);
    PUMP1 = 1;PUMP2 = 1;__delay_ms(7);
    PUMP1 = 0;PUMP2 = 0;__delay_ms(10);
    PUMP1 = 1;PUMP2 = 1;__delay_ms(6);
    PUMP1 = 0;PUMP2 = 0;__delay_ms(10);
    PUMP1 = 1;PUMP2 = 1;__delay_ms(5);
    PUMP1 = 0;PUMP2 = 0;__delay_ms(10);
    PUMP1 = 1;PUMP2 = 1;__delay_ms(4);
    PUMP1 = 0;PUMP2 = 0;__delay_ms(10);
    PUMP1 = 1;PUMP2 = 1;__delay_ms(3);
    PUMP1 = 0;PUMP2 = 0;__delay_ms(10);
    PUMP1 = 1;PUMP2 = 1;__delay_ms(2);
    PUMP1 = 0;PUMP2 = 0;__delay_ms(10);
    PUMP1 = 1;PUMP2 = 1;__delay_ms(1);
    PUMP1 = 0;PUMP2 = 0;
    return;
}