/*
 * led.c
 *
 *  Created on: 2016年8月4日
 *      Author: chen
 */

#include "led.h"

#define _LED1_PORT GPIOC
#define _LED1_PIN  GPIO_Pin_13

static void _TestTask( void const *pvParameters);

void Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //50M时钟速度
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    osThreadDef(test, _TestTask, osPriorityIdle, 0, 64);
    osThreadCreate(osThread(test), NULL);

#ifdef DEBUG
    Led_OnOff(true);
#endif
}

void Led_OnOff(bool on)
{
    if(on)
    {
        GPIO_ResetBits(_LED1_PORT, _LED1_PIN);
    }
    else
    {
        GPIO_SetBits(_LED1_PORT, _LED1_PIN);
    }
}

void Led_Trigger(void)
{
    uint8_t stat;

    stat = GPIO_ReadOutputDataBit(_LED1_PORT, _LED1_PIN);
    Led_OnOff(stat);
}


static void _TestTask( void const *pvParameters)
{

    while(1)
    {
        Led_Trigger();
        osDelay(500);
    }
}

