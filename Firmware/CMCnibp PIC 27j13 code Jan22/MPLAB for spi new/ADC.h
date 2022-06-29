/* 
 * File:   ADC.h
 * Author: Naveen Gangadharan
 *
 * Updated on 15 Dec 2021 Wednesday, 00:42
 */

#ifndef ADC_H
#define	ADC_H

#ifdef	__cplusplus
extern "C" {
#endif

    #define BATT_VOLT   8.5
    #define BATT_MIN    7.0

    extern float PRS_CALIB;
    void ADC_Init();
    float ADC_Read(unsigned char channel);
    unsigned char ADC_BatteryLevel();
    float ADC_ReadmmHg();
    void Pressure_Calibrate();
    
#ifdef	__cplusplus
}
#endif

#endif	/* ADC_H */

