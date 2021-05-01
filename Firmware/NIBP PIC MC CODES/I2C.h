/* 
 * File:   ADC.h
 * Author: Naveen Gangadharan
 *
 * Created on 12 August 2020 Wednesday, 00:42
 */


#ifndef I2C_H
#define	I2C_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifndef _XTAL_FREQ
#define _XTAL_FREQ  8000000
#endif    
#define I2C_100kHz  100000UL
    
    void I2C2_Init(const unsigned long c);
    void I2C2_Wait(void);
    void I2C2_Start(void);
    void I2C2_RepeatedStart(void);
    void I2C2_Stop(void);
    void I2C2_Write(unsigned char b);
    void I2C2_WriteMulti(unsigned char *b, unsigned char l);

#ifdef	__cplusplus
}
#endif

#endif	/* I2C_H */

