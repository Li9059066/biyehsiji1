#include "MQ7.h"	
#include "OLED.h"	 

#include <stdio.h>	
u8 times1;
void Adc_Init1()
{ 
 GPIO_InitTypeDef GPIO_Initstructre;     
 ADC_InitTypeDef ADC_InitStruct;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1,ENABLE);
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	 // 配置PA1为模拟输入
 GPIO_Initstructre.GPIO_Mode=GPIO_Mode_AIN; 
 GPIO_Initstructre.GPIO_Pin=GPIO_Pin_1;
	// ADC配置与MQ2类似
 GPIO_Initstructre.GPIO_Speed=GPIO_Speed_50MHz;
 GPIO_Init(GPIOA,&GPIO_Initstructre);
 GPIO_SetBits(GPIOA,GPIO_Pin_1);
 
 RCC_ADCCLKConfig(RCC_PCLK2_Div6);
 ADC_DeInit(ADC1);
 ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
 ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;
 ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
 ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;
 ADC_InitStruct.ADC_NbrOfChannel = 1;
 ADC_InitStruct.ADC_ScanConvMode = DISABLE;
 ADC_Init(ADC1,&ADC_InitStruct);
 ADC_Cmd(ADC1,ENABLE);
 ADC_ResetCalibration(ADC1);
 while(ADC_GetResetCalibrationStatus(ADC1));
 
 ADC_StartCalibration(ADC1);
 
 while(ADC_GetCalibrationStatus(ADC1));	
} 
	  
u16 Get_Adc1(u8 ch)
{
 ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_239Cycles5);

 ADC_SoftwareStartConvCmd(ADC1,ENABLE);

 while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));

 return ADC_GetConversionValue(ADC1);

}


u16 Get_Adc_Average1(u8 ch,u8 times1)
{
 u32 temp_val=0;
 u8 t;
 for(t=0;t<times1;t++)
  {
   temp_val+=Get_Adc1(ch);
   Delay_ms(5);
  }
 return temp_val/times1;

}
	
/*u16 MQ2_Value;
float  temp_one;
char strValue[20]={0};
char Dis_VA_Buf[20]={0};
void MQ2_Check(void)
{
	MQ2_Value=Get_Adc_Average(ADC_Channel_1,10)*100/4095;
	temp_one =(float) MQ2_Value/4096*3.3; 
	//sprintf(strValue,"AD_VO:%.2fV  ",temp_one);
	sprintf(Dis_VA_Buf,"Smoke: %d ",MQ2_Value);
	//OLED_ShowString(1,1,strValue);
	OLED_ShowString(3,1,Dis_VA_Buf);
	OLED_ShowString(3,11,"%");
	
	
		if(Flag==1)
		{
			USART1_RECEIVE();
		}
		else
		{
			OPEN_ALARM(MQ2_Value);
		}
}
*/
