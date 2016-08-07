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
}

static void _test(void)
{
#if TEST_ASSERT
	assert(false);
#endif
}

void vTaskA( void const *pvParameters)
{

    while(1)
    {
        Led_Trigger();
        osDelay(500);
    }
}

int main(int argc, char* argv[])
{
    _Init();
    Led_Init();
    Serial_Init();
    Audio_Init();

    DEBUG_MSG("\nSystem start.\n");
    _test();

    osThreadDef(defaultTask, vTaskA, osPriorityNormal, 0, 64);
    osThreadCreate(osThread(defaultTask), NULL);

    /* Start the scheduler. */
    osKernelStart();

    /* Will only get here if there was not enough heap space to create the
     idle task. */
    DEBUG_MSG("OS Failed!\n");
    Led_Trigger();
    exit(EXIT_FAILURE);

    return EXIT_FAILURE;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
