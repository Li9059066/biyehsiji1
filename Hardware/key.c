/**************************************************************************************
输入模式

    -输入浮空（GPIO_Mode_IN_FLOATING）

    -输入上拉(GPIO_Mode_IPU)

    -输入下拉(GPIO_Mode_IPD)

    -模拟输入(GPIO_Mode_AIN)

输出模式

    -开漏输出(GPIO_Mode_Out_OD)

    -开漏复用功能(GPIO_Mode_AF_OD)

    -推挽式输出(GPIO_Mode_Out_PP)

    -推挽式复用功能(GPIO_Mode_AF_PP)	 

**************************************************************************************/


#include "key.h"
#include "delay.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK Mini STM32开发板
//按键输入 驱动代码		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/3/06
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019




//按键初始化函数 
//PA15和PC5 设置成输入
void KEY_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能PORTA,PORTC时钟

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);//关闭jtag，使能SWD，可以用SWD模式调试

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1;       //PA4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;    //设置成上拉输入，平时为高，按下时低电平拉低
 	GPIO_Init(GPIOB, &GPIO_InitStructure);           //初始化GPIOA4
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4;        //PA5
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;     //设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);             //初始化GPIOA5
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_5;       //PC15
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;    //设置成上拉输入，平时为高，按下时低电平拉低
 	GPIO_Init(GPIOB, &GPIO_InitStructure);           //初始化GPIOC15
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;				    //触发引脚-->PC14 端口配置,由于是需要一个高电平脉冲，所以要设置为推挽式输出
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		  //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		  //IO口速度为50MHz
  GPIO_Init(GPIOB, &GPIO_InitStructure);					      //根据设定参数初始化GPIOC.14
	
	
	
} 
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//返回值：
//0，没有任何按键按下
//KEY0_PRES，KEY0按下
//KEY1_PRES，KEY1按下
//KEY2_PRES，KEY2按下
//KEY3_PRES，KEY3按下
//注意此函数有响应优先级,KEY0>KEY1>WK_UP!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		     
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||KEY3==0))
	{
//		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY0==0)return KEY0_PRES;
		else if(KEY1==0)return KEY1_PRES;
		else if(KEY2==0)return KEY2_PRES;
		else if(KEY3==0)return KEY3_PRES;
	}else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==1)key_up=1; 	     
	return 0;// 无按键按下
}












