#include "FMQ.h"
#include "stm32f10x.h" 
#include "delay.h" 
void mfq_Init(void)	// ��������ʼ��
{
	 // ����PB0Ϊ�������
	GPIO_InitTypeDef  GPIO_InitStructure; 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}

void fmq(float MQ2_Value)  // ��������Ũ�ȿ��Ʊ���
{
		if(MQ2_Value > 20)    // ������Ũ�ȳ���20%ʱ��������
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_0);//�͵�ƽ����
    }
		else
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_0);  // �ߵ�ƽ�رշ�����
		}
}


void beep_on(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_0);  // �ߵ�ƽ�򿪷����� 
}

void beep_off(void)
{
	  
	GPIO_ResetBits(GPIOB, GPIO_Pin_0);
}


void beep_alarm(uint8_t is_open, uint8_t cnt)
{
	if(is_open == 0)return;
	
	 for(uint8_t i = 0;i<cnt;i++ )
	{
		beep_on();
		Delay_ms(100);
		beep_off();   
		Delay_ms(100);
	}
}