/* 
 * File:   ADC.h
 * Author: Naveen Gangadharan
 *
 * Created on 12 August 2020 Wednesday, 00:42
 */


// Reference: Adafruit Industries SSD1306 OLED driver and graphics library.     *
//                                                                              *
// The driver is for I2C mode only.

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
   
        SSD1306_Command(SSD1306_DISPLAYOFF);                    
        SSD1306_Command(SSD1306_SETDISPLAYCLOCKDIV);            
        SSD1306_Command(0x80);                                  
        SSD1306_Command(SSD1306_SETMULTIPLEX);                  
        SSD1306_Command(0x1F);
        SSD1306_Command(SSD1306_SETDISPLAYOFFSET);              
        SSD1306_Command(0x0);                                   
        SSD1306_Command(SSD1306_SETSTARTLINE | 0x0);            
        SSD1306_Command(SSD1306_CHARGEPUMP);                    
        if (vccstate == SSD1306_EXTERNALVCC) { SSD1306_Command(0x10); }
        else { SSD1306_Command(0x14); }
        SSD1306_Command(SSD1306_MEMORYMODE);                   
        SSD1306_Command(0x00);                                 
        SSD1306_Command(SSD1306_SEGREMAP | 0x1);
        SSD1306_Command(SSD1306_COMSCANDEC);
        SSD1306_Command(SSD1306_SETCOMPINS);                    
        SSD1306_Command(0x02);
        SSD1306_Command(SSD1306_SETCONTRAST);                   
        SSD1306_Command(0x8F);
        SSD1306_Command(SSD1306_SETPRECHARGE);                  
        if (vccstate == SSD1306_EXTERNALVCC) { SSD1306_Command(0x22); }
        else { SSD1306_Command(0xF1); }
        SSD1306_Command(SSD1306_SETVCOMDETECT);                 
        SSD1306_Command(0x40);
        SSD1306_Command(SSD1306_DISPLAYALLON_RESUME);           
        SSD1306_Command(SSD1306_NORMALDISPLAY);                 
    #endif

    #if defined SSD1306_128_64
        // Init sequence for 128x64 OLED module
        SSD1306_Command(SSD1306_DISPLAYOFF);                    
        SSD1306_Command(SSD1306_SETDISPLAYCLOCKDIV);            
        SSD1306_Command(0x80);                                  
        SSD1306_Command(SSD1306_SETMULTIPLEX);                  
        SSD1306_Command(0x3F);
        SSD1306_Command(SSD1306_SETDISPLAYOFFSET);              
        SSD1306_Command(0x0);                                   
        SSD1306_Command(SSD1306_SETSTARTLINE | 0x0);            
        SSD1306_Command(SSD1306_CHARGEPUMP);                    
        if (vccstate == SSD1306_EXTERNALVCC) { SSD1306_Command(0x10); }
        else { SSD1306_Command(0x14); }
        SSD1306_Command(SSD1306_MEMORYMODE);                   
        SSD1306_Command(0x00);                                  
        SSD1306_Command(SSD1306_SEGREMAP | 0x1);
        SSD1306_Command(SSD1306_COMSCANDEC);
        SSD1306_Command(SSD1306_SETCOMPINS);                    
        SSD1306_Command(0x12);
        SSD1306_Command(SSD1306_SETCONTRAST);                   
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

    SSD1306_Command(SSD1306_DISPLAYON);

    __delay_ms(5);

    SSD1306_Command(SSD1306_SETLOWCOLUMN | 0x0);  
    SSD1306_Command(SSD1306_SETHIGHCOLUMN | 0x0);  
    SSD1306_Command(SSD1306_SETSTARTLINE | 0x0); 

    for (i = 0; i < SSD1306_LCDHEIGHT / 8; i++)
    {
        
        SSD1306_Command(0xB0 + i);
        SSD1306_Command(0);
        SSD1306_Command(0x10);

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

    SSD1306_Command(0x00);
    SSD1306_Command(0x10);
    SSD1306_Command(0xB0);

    for (i=0; i<(SSD1306_LCDHEIGHT/8); i++)
    {
       
        SSD1306_Command(0xB0 + i);
        SSD1306_Command(0);
        SSD1306_Command(0x10);

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
    
    SSD1306_Command(0xB0 + y);
    SSD1306_Command(x & 0xf);
    SSD1306_Command(0x10 | (x >> 4));

    I2C2_Start();
    I2C2_Write(SSD1306_I2C_ADDRESS);
    I2C2_Write(0x40);
    I2C2_WriteMulti(buffer, 8);
    I2C2_Stop();
}