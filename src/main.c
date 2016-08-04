//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include "main.h"
#include <stdlib.h>
#include "diag/Trace.h"

#include "led.h"
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

void TaskDelayMs(portTickType xTicksToDelay)
{
    vTaskDelay(xTicksToDelay / portTICK_RATE_MS);
}


void vTaskA( void *pvParameters)
{
    ON_DEBUG(uint i);
    uint td = *((uint *) pvParameters);
    TaskHandle_t myHandle;
    const char * myName;

    myHandle = xTaskGetCurrentTaskHandle();
    myName = pcTaskGetName(myHandle);

    while(1)
    {
        Led_Trigger();
        MDEBUG_COLOR(GREEN, "%s: %u\n", myName, i++);
        TaskDelayMs(td);
    }
}


int
main(int argc, char* argv[])
{
	static uint tad = 500;
	static uint tbd = 2000;

    _Init();
    Led_Init();
//    prvSetupHardware();
//    _SerialInit();

    DEBUG_MSG("\nSystem Start.\n");

//  testPrint();
//  while(1);

    xTaskCreate(vTaskA, "TaskA", 128, & tad, 5, NULL);
    xTaskCreate(vTaskA, "TaskB", 128, & tbd, 5, NULL);
//    xTaskCreate(vTaskA, "TaskC", 128, "TaskC", 5, NULL);

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
