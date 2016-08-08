/*
 * oscalls.c
 *
 *  Created on: 2016年8月5日
 *      Author: chenss
 */

#include "main.h"
#include "task.h"
#include "ExceptionHandlers.h"
#include <stdlib.h>

#if configCHECK_FOR_STACK_OVERFLOW
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	printf("Task: %s stack overflow!\n", pcTaskName);
}
#endif

void HardFault_Handler_C (ExceptionStackFrame* frame __attribute__((unused)),
        uint32_t lr __attribute__((unused)))
{
	puts("Hard Fault");
	exit(-1);
}

void UsageFault_Handler_C (ExceptionStackFrame* frame __attribute__((unused)),
        uint32_t lr __attribute__((unused)))
{
	puts("Usage Fault");
	exit(-1);
}

void BusFault_Handler_C (ExceptionStackFrame* frame __attribute__((unused)),
                    uint32_t lr __attribute__((unused)))
{
	puts("Bus Fault");
	exit(-1);
}

