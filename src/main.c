//
// This file is part of the GNU ARM Eclipse distribution.
// Copyright (c) 2014 Liviu Ionescu.
//

// ----------------------------------------------------------------------------

#include "main.h"
#include <stdlib.h>
#include "diag/Trace.h"
#include "task.h"

#include "led.h"
#include "serial.h"
#include "audio.h"

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


#define TEST_ASSERT 0

static void _Init(void)
{

    NVIC_PriorityGroupConfig( NVIC_PriorityGroup_4 );

    /* Configure HCLK clock as SysTick clock source. */
    SysTick_CLKSourceConfig( SysTick_CLKSource_HCLK );
}

static void _test(void)
{
#if TEST_ASSERT
	assert(false);
#endif
}

void vTaskA( void *pvParameters)
{
//    ON_DEBUG(uint i = 0);
    uint td = *((uint *) pvParameters);
//    ON_DEBUG(TaskHandle_t myHandle);
//    ON_DEBUG(const char * myName);
//
//    ON_DEBUG(myHandle = xTaskGetCurrentTaskHandle());
//    ON_DEBUG(myName = pcTaskGetName(myHandle));

    while(1)
    {
        Led_Trigger();
//        MDEBUG_COLOR(GREEN, "%s: %u\n", myName, i++);
//        ON_DEBUG(printf("TaskA: %u\n", i++));
        Utils_DelayMs(td);
    }
}

int main(int argc, char* argv[])
{
	static uint tad = 513;
//	static uint tbd = 987;
//	static uint tcd = 1743;

    _Init();
    Led_Init();
    Serial_Init();
    Audio_Init();

    DEBUG_MSG("\nSystem start.\n");
    _test();

    xTaskCreate(vTaskA, "TaskA", 128, & tad, 5, NULL);
    xTaskCreate(Audio_SampleTask, "Audio", 256, NULL, 4, NULL);

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
