/***************************************************
**HAL.c
**主要用于芯片硬件的内部外围和外部外围的初始化，两大INIT函数
**在MAIN中调用，使MAIN函数中尽量与硬件库无关
***************************************************/

#include "stm32f10x.h"


//各个内部硬件模块的配置函数
extern void GPIO_Configuration(void);			//GPIO
extern void TIM_Configuration(void);			//TIM
extern void NVIC_Configuration(void);			//NVIC
extern void IWDG_Configuration(void);			//IWDG
extern void USART_Configuration(void);			//串口
extern void ADC_DMA_Configuration(void);
extern void ADC_Configuration(void);
/*******************************
**函数名:ChipHalInit()
**功能:片内硬件初始化
*******************************/
void  ChipHalInit(void)
{
	//初始化中断源
	NVIC_Configuration();

	//初始化GPIO
	GPIO_Configuration();
		
	//初始化定时器
	TIM_Configuration();

		//初始化串口
//	USART_Configuration();

	ADC_Configuration();
//初始化独立看门狗
	IWDG_Configuration();
}


/*********************************
**函数名:ChipOutHalInit()
**功能:片外硬件初始化
*********************************/
void  ChipOutHalInit(void)
{
	
}
/***********************************************************************************
************************************************************************************
*******************DIY视界出品   http://59tiaoba.taobao.com*************************
************************************************************************************
*************************************************************************************/
