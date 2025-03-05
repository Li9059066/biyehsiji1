#include "MQ2.h"	
#include "OLED.h"	 
#include "FMQ.h"
#include <stdio.h>	
u8 times;
void Adc_Init()
{ 
	// 配置PA3为模拟输入
 GPIO_InitTypeDef GPIO_Initstructre;     
 ADC_InitTypeDef ADC_InitStruct;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1,ENABLE);
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
 GPIO_Initstructre.GPIO_Mode=GPIO_Mode_AIN; 
 GPIO_Initstructre.GPIO_Pin=GPIO_Pin_3;
 GPIO_Initstructre.GPIO_Speed=GPIO_Speed_50MHz;
 GPIO_Init(GPIOA,&GPIO_Initstructre);
 GPIO_SetBits(GPIOA,GPIO_Pin_3);
 
 RCC_ADCCLKConfig(RCC_PCLK2_Div6);
 ADC_DeInit(ADC1);
 ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
 ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right; // 数据右对齐
 ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
	  // ADC配置
 ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;// 独立模式
 ADC_InitStruct.ADC_NbrOfChannel = 1;
 ADC_InitStruct.ADC_ScanConvMode = DISABLE;// 单次转换
 ADC_Init(ADC1,&ADC_InitStruct);
 ADC_Cmd(ADC1,ENABLE);
 ADC_ResetCalibration(ADC1);
 while(ADC_GetResetCalibrationStatus(ADC1));
 
 ADC_StartCalibration(ADC1);
 
 while(ADC_GetCalibrationStatus(ADC1));	
} 
	  
u16 Get_Adc(u8 ch)
{
	 // 配置规则通道组
 ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_239Cycles5);
 // 启动软件转换
 ADC_SoftwareStartConvCmd(ADC1,ENABLE);
// 等待转换完成
 while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
 // 返回转换结果
 return ADC_GetConversionValue(ADC1);

}


u16 Get_Adc_Average(u8 ch,u8 times)
{
 u32 temp_val=0;
 u8 t;
	 // 多次采样取平均值
 for(t=0;t<times;t++)
  {
   temp_val+=Get_Adc(ch);
   Delay_ms(5); // 采样间隔
  }
 return temp_val/times;// 返回平均值

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
