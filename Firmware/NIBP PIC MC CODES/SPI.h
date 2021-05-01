/* 
 * File:   ADC.h
 * Author: Naveen Gangadharan
 *
 * Created on 12 August 2020 Wednesday, 00:42
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


