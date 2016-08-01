#include "stm32f10x.h"

RCC_ClocksTypeDef RCC_ClockFreq;

void RCC_Configuration(void)
{
	SystemInit();//源自system_stm32f10x.c文件,只需要调用此函数,则可完成RCC的配置.具体请看2_RCC

	/**************************************************
	获取RCC的信息,调试用
	请参考RCC_ClocksTypeDef结构体的内容,当时钟配置完成后,
	里面变量的值就直接反映了器件各个部分的运行频率
	***************************************************/
	RCC_GetClocksFreq(&RCC_ClockFreq);
	
	/* 这个配置可使外部晶振停振的时候,产生一个NMI中断,不需要用的可屏蔽掉*/
	//RCC_ClockSecuritySystemCmd(ENABLE);

   	//SYSTICK分频--1ms的系统时钟中断
//	if (SysTick_Config(SystemFrequency / 1000))
//  	{ 
//  	  	/* Capture error */ 
//    	while (1);
//  	}

}
/********************************************
**函数名:SysTickDelay
**功能:使用系统时钟的硬延迟
**注意事项:一般地,不要在中断中调用本函数,否则会存在重入问题.另外如果屏蔽了全局中断,则不要使用此函数
********************************************/
//volatile u16 Timer1;
//void SysTickDelay(u16 dly_ms)
//{
//	Timer1=dly_ms;
//	while(Timer1);
//}
