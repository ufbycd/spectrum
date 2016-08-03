/*
 * main.c
 *
 *  Created on: 2013-6-1
 *      Author: chenss
 */

//#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "main.h"
#include "task.h"
#include "semphr.h"
//#include "test.h"

void usart2_init();
static void prvSetupHardware(void);

static xSemaphoreHandle _mx_print;

void uart_putchar(char c)
{
	while ((USART2->SR & USART_FLAG_TC) == (uint16_t)RESET);
	USART2->DR = (c & (uint16_t)0x01FF);
}

static void _printfSaveInit(void)
{
	_mx_print = xSemaphoreCreateMutex();
}

int printSafe(const char *fmt, ...)
{
	int i;
	va_list args;

	va_start(args, fmt);

	xSemaphoreTake(_mx_print, 1000);
	i = vprintf(fmt, args);
	xSemaphoreGive(_mx_print);

	va_end(args);

	return i;
}

void vApplicationStackOverflowHook( xTaskHandle xTask, signed portCHAR *pcTaskName )
{
	DEBUG_MSG("ERR: Stack Overflow at Task: %s!\n", pcTaskName);
}

void vApplicationMallocFailedHook( void )
{
	DEBUG_MSG("ERR: Malloc Failed !\n");
}

void vTaskDelayMs(portTickType xTicksToDelay)
{
	vTaskDelay(xTicksToDelay / portTICK_RATE_MS);
}

void vTaskA( void *pvParameters)
{
	int i = 0;
	const char *name = pvParameters;


	while(1)
	{
		MDEBUG_COLOR(GREEN, "%s: %d\n", name, i++);
		vTaskDelayMs(400);
	}
}

int main(void)
{
	prvSetupHardware();
	usart2_init();
	_printfSaveInit();

	DEBUG_MSG("\nSystem Start.\n");

//	testPrint();
//	while(1);

	xTaskCreate(vTaskA, "TaskA", 128, "TaskA", 5, NULL);
	xTaskCreate(vTaskA, "TaskB", 128, "TaskB", 5, NULL);
	xTaskCreate(vTaskA, "TaskC", 128, "TaskC", 5, NULL);

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was not enough heap space to create the
	 idle task. */
	DEBUG_MSG("OS Failed!\n");
	exit(-1);

	return EXIT_FAILURE;
}

static void prvSetupHardware(void)
{
	/* Set the Vector Table base address at 0x08000000 */
	NVIC_SetVectorTable( NVIC_VectTab_FLASH, 0x0 );

	NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

	/* Configure HCLK clock as SysTick clock source. */
	SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
}

void usart2_init()
{
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl =
			USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(
			RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO,
			ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

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
	USART_Init(USART1, &USART_InitStructure);

	/* Enable USART */
	USART_Cmd(USART1, ENABLE);
}

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	DEBUG_MSG("Wrong parameters value: %s:%lu\r\n", file, line);

  /* Infinite loop */
  while (1);
}

/**
 * @brief
 */
void HardFault_Handler(void)
{
	uint32_t r_sp;

	r_sp = __get_PSP();
	DEBUG_MSG("IRQ: HardFault, PSP = %#lx\n", r_sp);

	while(1);
}

void MemManage_Handler(void)
{
	DEBUG_MSG("IRQ: MemManager\n");
	while(1);
}

void BusFault_Handler(void)
{
	DEBUG_MSG("IRQ: BusFault\n");
	while(1);
}

void UsageFault_Handler(void)
{
	DEBUG_MSG("IRQ: UsageFault\n");
	while(1);
}

