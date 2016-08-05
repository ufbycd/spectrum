/*
 * serial.c
 *
 *  Created on: 2016年8月5日
 *      Author: chenss
 */

#include "serial.h"



void Serial_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef uartConfig;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    USART_StructInit(& uartConfig);
	uartConfig.USART_BaudRate = 115200;


	/* Configure USART Tx as push-pull */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* USART configuration */
	USART_Init(USART1, &uartConfig);

	/* Enable USART */
	USART_Cmd(USART1, ENABLE);
}

char Serial_GetChar(void)
{
	return '\0';
}

void Serial_PutChar(char c)
{
	if(c == '\n')
	{
		Serial_PutChar('\r');
	}

	USART_SendData(USART1, c);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
	{
	}
}


