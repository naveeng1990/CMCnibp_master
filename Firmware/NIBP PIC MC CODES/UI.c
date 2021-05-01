/* 
 * File:   ADC.h
 * Author: Naveen Gangadharan
 *
 * Created on 12 August 2020 Wednesday, 00:42
 */


#include <xc.h>
#include <stdlib.h>
#include "UI.h"
#include "I2C.h"
#include "SSD1306.h"
#include "OLED.h"

extern uint8_t m_col;
extern uint8_t m_row;
extern uint8_t m_font;
extern uint8_t m_flags;

void UI_Clear()
{
    OLED_Clear(0,0,SSD1306_LCDWIDTH,SSD1306_LCDHEIGHT);
}

void UI_LowBattery()
{
    OLED_BatteryLevel(0);
    m_font = FONT_SIZE_SMALL;
    OLED_SetCursor(28,4);
    OLED_Print("Low Battery!",12);
}

void UI_Ready(int loaded)
{
    m_font = FONT_SIZE_MEDIUM;
    OLED_SetCursor(37,2);
    OLED_Print("READY!",6);
    m_font = FONT_SIZE_SMALL;
    OLED_SetCursor(0,6);
    OLED_Print("A) Run: ",8);
    if(loaded == 0)
    OLED_Print("Default",7);
    else
    {
        OLED_PrintInt(loaded,3);
        OLED_Print(" mmHg",5);
    }
    OLED_SetCursor(0,7);
    OLED_Print("B) Run: ",8);
    
    OLED_Print("Ramp",4);
}

void UI_Proceed(int *levels)
{
    int lev_order;
    m_font = FONT_SIZE_SMALL;
    
    for(lev_order=1; lev_order<7; lev_order++)
    {
        OLED_SetCursor(0,lev_order);
        OLED_PrintInt(lev_order,2);
        OLED_Write(')');
        OLED_PrintInt(*levels,3);
        levels++;
    }
    for(lev_order; lev_order<13; lev_order++)
    {
        OLED_SetCursor(46,lev_order-6);
        OLED_PrintInt(lev_order,2);
        OLED_Write(')');
        OLED_PrintInt(*levels,3);
        levels++;
    }
    for(lev_order; lev_order<17; lev_order++)
    {
        OLED_SetCursor(91,lev_order-12);
        OLED_PrintInt(lev_order,2);
        OLED_Write(')');
        OLED_PrintInt(*levels,3);
        levels++;
    }
    OLED_SetCursor(0,7);
    OLED_Print("Press (A) to proceed",20);
}

void UI_RampProceed()
{
    m_font = FONT_SIZE_SMALL;
    OLED_SetCursor(0,3);
    OLED_Print("Ramp sequence will be",21);
    OLED_SetCursor(0,4);
    OLED_Print("      initiated.",16);
    OLED_SetCursor(0,7);
    OLED_Print("Press (A) to proceed",20);
}

void UI_SetPress(int prs)
{
    m_font = FONT_SIZE_SMALL;
    OLED_ClearLine(1);
    OLED_SetCursor(40,1);
    OLED_Print("SET: ",5);
    OLED_PrintInt(prs,3);
    m_font = FONT_SIZE_MEDIUM;
    OLED_SetCursor(46,6);
    OLED_Print("mmHg",4);
}

void UI_ReadPress(int prs)
{
    m_font = FONT_SIZE_MEDIUM;
    OLED_ClearLine(2);
    OLED_ClearLine(3);
    OLED_ClearLine(4);
    OLED_ClearLine(5);
    OLED_SetCursor(50,3);
    OLED_PrintLong(prs,1);
}
