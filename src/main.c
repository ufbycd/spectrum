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
#include "LedMatrix.h"
#include "keypad.h"

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
// FIXME 上下两屏的第一行的显示亮度比其他的低
// TODO 实现软件校准功能来去除音频采集时的噪声
// TODO 检测环境光强度，并据此调节适当的显示亮度
// TODO 软件实现自动增益控制（AGC）功能
// TODO 添加全局事件广播功能
// TODO 添加MIC音源输入
// TODO 根据节奏动态调节显示不同的频谱颜色
// TODO 实现极光式的随节奏变化的动画显示
// TODO 添加开机动画
// TODO 实现音源无声音后开始显示个性动画，直到音源发声后才恢复频谱显示

// ----- main() ---------------------------------------------------------------

// Sample pragmas to cope with warnings. Please note the related line at
// the end of this function, used to pop the compiler diagnostics status.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"


#define TEST_ASSERT 0
#define TEST_TASK   1

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

int main(int argc, char* argv[])
{
    _Init();
    Led_Init();
    Keypad_Init();
    Serial_Init();
    Audio_Init();
    LedMatrix_Init();

    DEBUG_MSG("\nSystem start.\n");
    _test();

    /* Start the scheduler. */
    osKernelStart();

    /* Will only get here if there was not enough heap space to create the
     idle task. */
    DEBUG_MSG("OS Failed!\n");
    exit(EXIT_FAILURE);

    return EXIT_FAILURE;
}

#pragma GCC diagnostic pop

// ----------------------------------------------------------------------------
