/*
 * serial.c
 *
 *  Created on: 2016年8月5日
 *      Author: chenss
 */

#include "serial.h"
#include "circular_buffer.h"
#include "semphr.h"

#define _TX_BUFFER_SIZE 128u


static uint8_t _txBuf[_TX_BUFFER_SIZE];
static cbuf_t _txCtrl;
static SemaphoreHandle_t _sempHandle;

void Serial_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef uartConfig;
	NVIC_InitTypeDef nvicConfig;

	CBUF_Init(&_txCtrl, _txBuf, sizeof(_txBuf));
	_sempHandle = xSemaphoreCreateCounting(_TX_BUFFER_SIZE, _TX_BUFFER_SIZE);
	configASSERT(_sempHandle);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	nvicConfig.NVIC_IRQChannel = USART1_IRQn;
	nvicConfig.NVIC_IRQChannelPreemptionPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY + 1;
	nvicConfig.NVIC_IRQChannelSubPriority = 8;
	nvicConfig.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(& nvicConfig);

	USART_StructInit(&uartConfig);
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

//	if(xSemaphoreTake(_sempHandle, 2))
	{
		if(c == '\n')
		{
			Serial_PutChar('\r');
		}

#if 0
		CBUF_Write(& _txCtrl, & c);
		if(USART_GetFlagStatus(USART1, USART_FLAG_TC) == SET)
		{
			USART_ClearITPendingBit(USART1, USART_IT_TXE);
			USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
		}
#else
	USART_SendData(USART1, c);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
#endif
	}
}

void USART1_IRQHandler(void)
{
	elem_t c;

	if(CBUF_Read(& _txCtrl, & c) == 0)
	{
		USART_SendData(USART1, (uint16_t) c);
//		xSemaphoreGiveFromISR(_sempHandle, NULL);
	}
	else
	{
		USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
	}
}


