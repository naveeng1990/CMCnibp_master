/* 
 * File:   SPI.h
 * Author: Naveen Gangadharan
 *
 * Updated on 12 Dec 2021 Wednesday, 00:42
 */

#ifndef SPI_H
#define	SPI_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef _XTAL_FREQ
#define _XTAL_FREQ  8000000
#endif    
#define I2C_100kHz  100000UL
    void SPI1_Init(void);
    void __interrupt() SPI1_INT(void);
#ifdef	__cplusplus
}
#endif

#endif	/* SPI_H */


