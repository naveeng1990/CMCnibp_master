/* 
 * File:   OLED.h
 * Author: Naveen Gangadharan
 *
 * Updated on 12 Dec 2021 Wednesday, 00:42
 */

#ifndef OLED_H
#define	OLED_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "SSD1306.h"
    
typedef enum {
    FONT_SIZE_SMALL = 0,
    FONT_SIZE_MEDIUM,
    FONT_SIZE_LARGE,
    FONT_SIZE_XLARGE
} FONT_SIZE;

#define FLAG_PAD_ZERO 1
#define FLAG_PIXEL_DOUBLE_H 2
#define FLAG_PIXEL_DOUBLE_V 4
#define FLAG_PIXEL_DOUBLE (FLAG_PIXEL_DOUBLE_H | FLAG_PIXEL_DOUBLE_V)

extern uint8_t m_col;
extern uint8_t m_row;
extern uint8_t m_font;
extern uint8_t m_flags;

extern const unsigned char font5x8[][5];
extern const unsigned char digits8x8[][8] ;
extern const unsigned char digits16x16[][32];
extern const unsigned char digits16x24[][48];
extern const unsigned char font8x16_doslike[][16];
extern const unsigned char font8x16_terminal[][16];

void OLED_BatteryLevel(unsigned char level);
void OLED_Print(uint8_t *c, uint8_t l);
void OLED_PrintInt(uint16_t value, uint8_t padding);
void OLED_PrintLong(unsigned long value, uint8_t padding);
void OLED_SetCursor(uint8_t column, uint8_t line);
unsigned OLED_Write(uint8_t c);
void OLED_WriteDigit(uint8_t n);
void OLED_ClearLine(uint8_t line);
void OLED_Clear(uint8_t x, uint8_t y, uint8_t width, uint8_t height);
void OLED_SetContrast(uint8_t Contrast);

#ifdef	__cplusplus
}
#endif

#endif	/* OLED_H */

