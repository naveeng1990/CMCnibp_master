/* 
 * File:   SSD1306.h
 * Author: Naveen Gangadharan
 *
 * Updated on 12 Dec 2021 Wednesday, 00:42
 */

#ifndef SSD1306_H
#define	SSD1306_H

#ifdef	__cplusplus
extern "C" {
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
    
#define SSD1306_I2C_ADDRESS   (0x3C << 1)//0x78	// 011110+SA0+RW - 0x3C or 0x3D
// Address for 128x32 is 0x3D (default) or 0x3C (if SA0 is grounded)

   #define SSD1306_128_64
//   #define SSD1306_128_32
/*=========================================================================*/

#if defined SSD1306_128_64 && defined SSD1306_128_32
  #error "Only one SSD1306 display can be specified at once in SSD1306.h"
#endif
#if !defined SSD1306_128_64 && !defined SSD1306_128_32
  #error "At least one SSD1306 display must be specified in SSD1306.h"
#endif

#if defined SSD1306_128_64
  #define SSD1306_LCDWIDTH                  128
  #define SSD1306_LCDHEIGHT                 64
#endif
#if defined SSD1306_128_32
  #define SSD1306_LCDWIDTH                  128
  #define SSD1306_LCDHEIGHT                 32
#endif

#define SSD1306_SETCONTRAST                 0x81
#define SSD1306_DISPLAYALLON_RESUME         0xA4
#define SSD1306_DISPLAYALLON                0xA5
#define SSD1306_NORMALDISPLAY               0xA6
#define SSD1306_INVERTDISPLAY               0xA7
#define SSD1306_DISPLAYOFF                  0xAE
#define SSD1306_DISPLAYON                   0xAF

#define SSD1306_SETDISPLAYOFFSET            0xD3
#define SSD1306_SETCOMPINS                  0xDA

#define SSD1306_SETVCOMDETECT               0xDB

#define SSD1306_SETDISPLAYCLOCKDIV          0xD5
#define SSD1306_SETPRECHARGE                0xD9

#define SSD1306_SETMULTIPLEX                0xA8

#define SSD1306_SETLOWCOLUMN                0x00
#define SSD1306_SETHIGHCOLUMN               0x10

#define SSD1306_SETSTARTLINE                0x40

#define SSD1306_MEMORYMODE                  0x20

#define SSD1306_COMSCANINC                  0xC0
#define SSD1306_COMSCANDEC                  0xC8

#define SSD1306_SEGREMAP                    0xA0

#define SSD1306_CHARGEPUMP                  0x8D

#define SSD1306_EXTERNALVCC                 0x1
#define SSD1306_SWITCHCAPVCC                0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL             0x2F
#define SSD1306_DEACTIVATE_SCROLL           0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA    0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL     0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL      0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL    0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL     0x2A


  void SSD1306_Begin(uint8_t switchvcc);
  void SSD1306_Command(uint8_t c);
  void SSD1306_Data(uint8_t c);

  void SSD1306_InvertDisplay(uint8_t i);
  void SSD1306_Draw8x8(uint8_t* buffer, uint8_t x, uint8_t y);

  void SSD1306_StartScrollRight(uint8_t start, uint8_t stop);
  void SSD1306_StartScrollLeft(uint8_t start, uint8_t stop);

  void SSD1306_StartScrollDiagRight(uint8_t start, uint8_t stop);
  void SSD1306_StartScrollDiagLeft(uint8_t start, uint8_t stop);
  void SSD1306_StopScroll(void);

  void SSD1306_Fill(unsigned char dat);
  void SSD1306_ClearBuffer();

#ifdef	__cplusplus
}
#endif

#endif	/* SSD1306_H */

