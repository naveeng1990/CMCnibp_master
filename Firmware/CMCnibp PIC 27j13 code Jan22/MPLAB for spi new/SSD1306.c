/* 
 * File:   SSD1306.c
 * Author: Naveen Gangadharan
 *
 * Updated on 12 Dec 2021 Wednesday, 00:42
 */

#include <xc.h>
#include <stdlib.h>
#include "I2C.h"
#include "SSD1306.h"


void SSD1306_Begin(uint8_t vccstate)
{
    uint8_t i, j, k;
    I2C2_Init(I2C_100kHz);
    __delay_ms(10);
    #if defined SSD1306_128_32
        // Init sequence for 128x32 OLED module
        SSD1306_Command(SSD1306_DISPLAYOFF);                    // 0xAE
        SSD1306_Command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
        SSD1306_Command(0x80);                                  // the suggested ratio 0x80
        SSD1306_Command(SSD1306_SETMULTIPLEX);                  // 0xA8
        SSD1306_Command(0x1F);
        SSD1306_Command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
        SSD1306_Command(0x0);                                   // no offset
        SSD1306_Command(SSD1306_SETSTARTLINE | 0x0);            // line #0
        SSD1306_Command(SSD1306_CHARGEPUMP);                    // 0x8D
        if (vccstate == SSD1306_EXTERNALVCC) { SSD1306_Command(0x10); }
        else { SSD1306_Command(0x14); }
        SSD1306_Command(SSD1306_MEMORYMODE);                    // 0x20
        SSD1306_Command(0x00);                                  // 0x0 act like ks0108
        SSD1306_Command(SSD1306_SEGREMAP | 0x1);
        SSD1306_Command(SSD1306_COMSCANDEC);
        SSD1306_Command(SSD1306_SETCOMPINS);                    // 0xDA
        SSD1306_Command(0x02);
        SSD1306_Command(SSD1306_SETCONTRAST);                   // 0x81
        SSD1306_Command(0x8F);
        SSD1306_Command(SSD1306_SETPRECHARGE);                  // 0xd9
        if (vccstate == SSD1306_EXTERNALVCC) { SSD1306_Command(0x22); }
        else { SSD1306_Command(0xF1); }
        SSD1306_Command(SSD1306_SETVCOMDETECT);                 // 0xDB
        SSD1306_Command(0x40);
        SSD1306_Command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
        SSD1306_Command(SSD1306_NORMALDISPLAY);                 // 0xA6
    #endif

    #if defined SSD1306_128_64
        // Init sequence for 128x64 OLED module
        SSD1306_Command(SSD1306_DISPLAYOFF);                    // 0xAE
        SSD1306_Command(SSD1306_SETDISPLAYCLOCKDIV);            // 0xD5
        SSD1306_Command(0x80);                                  // the suggested ratio 0x80
        SSD1306_Command(SSD1306_SETMULTIPLEX);                  // 0xA8
        SSD1306_Command(0x3F);
        SSD1306_Command(SSD1306_SETDISPLAYOFFSET);              // 0xD3
        SSD1306_Command(0x0);                                   // no offset
        SSD1306_Command(SSD1306_SETSTARTLINE | 0x0);            // line #0
        SSD1306_Command(SSD1306_CHARGEPUMP);                    // 0x8D
        if (vccstate == SSD1306_EXTERNALVCC) { SSD1306_Command(0x10); }
        else { SSD1306_Command(0x14); }
        SSD1306_Command(SSD1306_MEMORYMODE);                    // 0x20
        SSD1306_Command(0x00);                                  // 0x0 act like ks0108
        SSD1306_Command(SSD1306_SEGREMAP | 0x1);
        SSD1306_Command(SSD1306_COMSCANDEC);
        SSD1306_Command(SSD1306_SETCOMPINS);                    // 0xDA
        SSD1306_Command(0x12);
        SSD1306_Command(SSD1306_SETCONTRAST);                   // 0x81
        if (vccstate == SSD1306_EXTERNALVCC) { SSD1306_Command(0x9F); }
        else { SSD1306_Command(0xCF); }
        SSD1306_Command(SSD1306_SETPRECHARGE);                  // 0xd9
        if (vccstate == SSD1306_EXTERNALVCC) { SSD1306_Command(0x22); }
        else { SSD1306_Command(0xF1); }
        SSD1306_Command(SSD1306_SETVCOMDETECT);                 // 0xDB
        SSD1306_Command(0x40);
        SSD1306_Command(SSD1306_DISPLAYALLON_RESUME);           // 0xA4
        SSD1306_Command(SSD1306_NORMALDISPLAY);                 // 0xA6
    #endif

    SSD1306_Command(SSD1306_DISPLAYON);//--turn on oled panel

    // clear screen
    __delay_ms(5);

    SSD1306_Command(SSD1306_SETLOWCOLUMN | 0x0);  // low col = 0
    SSD1306_Command(SSD1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
    SSD1306_Command(SSD1306_SETSTARTLINE | 0x0); // line #0

    for (i = 0; i < SSD1306_LCDHEIGHT / 8; i++)
    {
        // send a bunch of data in one xmission
        SSD1306_Command(0xB0 + i);//set page address
        SSD1306_Command(0);//set lower column address
        SSD1306_Command(0x10);//set higher column address

        for(j = 0; j < 8; j++)
        {
            I2C2_Start();
            I2C2_Write(SSD1306_I2C_ADDRESS);
            I2C2_Write(0x40);
            for (k = 0; k < SSD1306_LCDWIDTH / 8; k++) { I2C2_Write(0); }
            I2C2_Stop();
        }
    }
}


void SSD1306_InvertDisplay(uint8_t i)
{
    if (i) { SSD1306_Command(SSD1306_INVERTDISPLAY); } 
    else { SSD1306_Command(SSD1306_NORMALDISPLAY); }
}

void SSD1306_Command(uint8_t c)
{
    // I2C
    I2C2_Start();
    I2C2_Write(SSD1306_I2C_ADDRESS);
    I2C2_Write(0x00);
    I2C2_Write(c);
    I2C2_Stop();
}

// startscrollright
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void SSD1306_StartScrollRight(uint8_t start, uint8_t stop)
{
	SSD1306_Command(SSD1306_RIGHT_HORIZONTAL_SCROLL);
	SSD1306_Command(0X00);
	SSD1306_Command(start);
	SSD1306_Command(0X00);
	SSD1306_Command(stop);
	SSD1306_Command(0X01);
	SSD1306_Command(0XFF);
	SSD1306_Command(SSD1306_ACTIVATE_SCROLL);
}

// startscrollleft
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void SSD1306_StartScrollLeft(uint8_t start, uint8_t stop)
{
	SSD1306_Command(SSD1306_LEFT_HORIZONTAL_SCROLL);
	SSD1306_Command(0X00);
	SSD1306_Command(start);
	SSD1306_Command(0X00);
	SSD1306_Command(stop);
	SSD1306_Command(0X01);
	SSD1306_Command(0XFF);
	SSD1306_Command(SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagright
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void SSD1306_StartScrollDiagRight(uint8_t start, uint8_t stop)
{
	SSD1306_Command(SSD1306_SET_VERTICAL_SCROLL_AREA);
	SSD1306_Command(0X00);
	SSD1306_Command(SSD1306_LCDHEIGHT);
	SSD1306_Command(SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL);
	SSD1306_Command(0X00);
	SSD1306_Command(start);
	SSD1306_Command(0X00);
	SSD1306_Command(stop);
	SSD1306_Command(0X01);
	SSD1306_Command(SSD1306_ACTIVATE_SCROLL);
}

// startscrolldiagleft
// Activate a diagonal scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void SSD1306_StartScrollDiagLeft(uint8_t start, uint8_t stop)
{
	SSD1306_Command(SSD1306_SET_VERTICAL_SCROLL_AREA);
	SSD1306_Command(0X00);
	SSD1306_Command(SSD1306_LCDHEIGHT);
	SSD1306_Command(SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL);
	SSD1306_Command(0X00);
	SSD1306_Command(start);
	SSD1306_Command(0X00);
	SSD1306_Command(stop);
	SSD1306_Command(0X01);
	SSD1306_Command(SSD1306_ACTIVATE_SCROLL);
}

void SSD1306_StopScroll(void)
{
	SSD1306_Command(SSD1306_DEACTIVATE_SCROLL);
}

void SSD1306_Data(uint8_t c)
{
    // I2C
    I2C2_Start();
    I2C2_Write(SSD1306_I2C_ADDRESS);
    I2C2_Write(0x40);
    I2C2_Write(c);
    I2C2_Stop();
}

void SSD1306_Fill(unsigned char dat)
{
    uint8_t i, j, k;

    SSD1306_Command(0x00);//set lower column address
    SSD1306_Command(0x10);//set higher column address
    SSD1306_Command(0xB0);//set page address

    for (i=0; i<(SSD1306_LCDHEIGHT/8); i++)
    {
        // send a bunch of data in one xmission
        SSD1306_Command(0xB0 + i);//set page address
        SSD1306_Command(0);//set lower column address
        SSD1306_Command(0x10);//set higher column address

        for(j = 0; j < 8; j++)
        {
            I2C2_Start();
            I2C2_Write(SSD1306_I2C_ADDRESS);
            I2C2_Write(0x40);
            for (k = 0; k < 16; k++) { I2C2_Write(dat); }
            I2C2_Stop();
        }
    }
}

void SSD1306_Draw8x8(uint8_t* buffer, uint8_t x, uint8_t y)
{
    // send a bunch of data in one xmission
    SSD1306_Command(0xB0 + y);//set page address
    SSD1306_Command(x & 0xf);//set lower column address
    SSD1306_Command(0x10 | (x >> 4));//set higher column address

    I2C2_Start();
    I2C2_Write(SSD1306_I2C_ADDRESS);
    I2C2_Write(0x40);
    I2C2_WriteMulti(buffer, 8);
    I2C2_Stop();
}