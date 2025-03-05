#ifndef __MQ7_H
#define __MQ7_H	 
#include "stm32f10x.h"                  // Device header
#include "Delay.h"
void Adc_Init1(void);
u16  Get_Adc1(u8 ch); 
u16 Get_Adc_Average1(u8 ch,u8 times); 


#endif

