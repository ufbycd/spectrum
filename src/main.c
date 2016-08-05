//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include "main.h"
#include <stdlib.h>
#include "diag/Trace.h"

#include "led.h"
#include "serial.h"
#include "task.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F1 empty sample (trace via NONE).
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


static void _Init(void)
{

    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

    /* Configure HCLK clock as SysTick clock source. */
    SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
}

inline void TaskDelayMs(portTickType xTicksToDelay)
{
    vTaskDelay(xTicksToDelay / portTICK_RATE_MS);
}


void vTaskA( void *pvParameters)
{
    ON_DEBUG(uint i = 0);
    uint td = *((uint *) pvParameters);
    ON_DEBUG(TaskHandle_t myHandle);
    ON_DEBUG(const char * myName);

    ON_DEBUG(myHandle = xTaskGetCurrentTaskHandle());
    ON_DEBUG(myName = pcTaskGetName(myHandle));

    while(1)
    {
        Led_Trigger();
//        MDEBUG_COLOR(GREEN, "%s: %u\n", myName, i++);
        ON_DEBUG(printf("%s: %u\n", myName, i++));
        TaskDelayMs(td);
    }
}

#if configCHECK_FOR_STACK_OVERFLOW
void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
	ON_DEBUG(printf("Task: %s stack overflow!\n", pcTaskName));
}
#endif


int
main(int argc, char* argv[])
{
	static uint tad = 513;
	static uint tbd = 987;
	static uint tcd = 1743;

    _Init();
    Led_Init();
    Serial_Init();

    DEBUG_MSG("\nSystem start.\n");

    xTaskCreate(vTaskA, "TaskA", 256, & tad, 5, NULL);
    xTaskCreate(vTaskA, "TaskB", 128, & tbd, 5, NULL);
//    xTaskCreate(vTaskA, "TaskC", 128, & tcd, 5, NULL);

    /* Start the scheduler. */
    vTaskStartScheduler();

    /* Will only get here if there was not enough heap space to create the
     idle task. */
    DEBUG_MSG("OS Failed!\n");
    Led_Trigger();
    exit(-1);

    return EXIT_FAILURE;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
