/* 
 * File:   ADC.h
 * Author: Naveen Gangadharan
 *
 * Created on 12 August 2020 Wednesday, 00:42
 */


#ifndef UI_H
#define	UI_H

#ifdef	__cplusplus
extern "C" {
#endif


    
void UI_Clear();
void UI_LowBattery();
void UI_Ready(int loaded);
void UI_Proceed(int *levels);
void UI_RampProceed();
void UI_SetPress(int prs);
void UI_ReadPress(int prs);

#ifdef	__cplusplus
}
#endif

#endif	/* UI_H */

