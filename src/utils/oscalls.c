/*
 * oscalls.c
 *
 *  Created on: 2016年8月5日
 *      Author: chenss
 */

#include "main.h"
#include "task.h"


#if configCHECK_FOR_STACK_OVERFLOW
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	DEBUG_MSG("Task: %s stack overflow!\n", pcTaskName);
}
#endif

void Utils_DelayMs(uint32_t ms)
{
	vTaskDelay(ms / portTICK_RATE_MS);
}

