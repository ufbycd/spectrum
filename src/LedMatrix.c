/*
 * LedMatrix.c
 *
 *  Created on: 2016年8月3日
 *      Author: chen
 */


#include "LedMatrix.h"


#define _MATRIX_COLUMN_COUNT 64
#define _MATRIX_ROW_COUNT    32
#define _MATRIX_COLOR_COUNT  3

// 显示时所需扫描的行数（上下两屏同时扫描）
#define _MATRIX_LINE_SCAN_COUNT (_MATRIX_ROW_COUNT / 2)

// 显示帧频率 Hz
#define _MATRIX_FRAME_FREQ 120

// 行扫描信号频率 Hz
#define _MATRIX_LINE_SCAN_FREQ (_MATRIX_FRAME_FREQ * _MATRIX_LINE_SCAN_COUNT)

#define _ADDR_PORT GPIOB

#define _IO_EN   (GPIOB, GPIO_Pin_11)
#define _IO_LAT  (GPIOA, GPIO_Pin_5)
#define _IO_CLK  (GPIOA, GPIO_Pin_6)
#define _IO_A    (_ADDR_PORT, GPIO_Pin_12)
#define _IO_B    (_ADDR_PORT, GPIO_Pin_13)
#define _IO_C    (_ADDR_PORT, GPIO_Pin_14)
#define _IO_D    (_ADDR_PORT, GPIO_Pin_15)
#define _IO_DR1  (GPIOA, GPIO_Pin_15)
#define _IO_DB1  (GPIOB, GPIO_Pin_3)
#define _IO_DG1  (GPIOB, GPIO_Pin_10)
#define _IO_DR2  (GPIOB, GPIO_Pin_6)
#define _IO_DB2  (GPIOB, GPIO_Pin_7)
#define _IO_DG2  (GPIOB, GPIO_Pin_8)
#define _IO_OE	  _IO_EN

#define _PORT(p, n) p
#define _PIN(p, n)  n

#define PORT(io) _PORT io
#define PIN(io)  _PIN io
#define IO_SET(io) PORT(io)->BSRR = PIN(io)
#define IO_CLR(io) PORT(io)->BRR = PIN(io)

#define MATRIX_ENABLE()  IO_CLR(_IO_EN)
#define MATRIX_DISABLE() IO_SET(_IO_EN)

#define MATRIX_SCAN_ON_LINE(n) BITS_SET(_ADDR_PORT->ODR, 0xf << 12, n << 12)

static volatile unsigned int _currentScanLine;

static void _GpioInit(void);
static void _FrameTimerInit(void);
static void _AnimationTask(void const *args);

static osThreadId _animationTid;

void LedMatrix_Init(void)
{
	_currentScanLine = 0;

	_GpioInit();
	_FrameTimerInit();

    osThreadDef(audio, _AnimationTask, osPriorityRealtime, 0, 128);
    _animationTid = osThreadCreate(osThread(audio), NULL);
}

static void _GpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //50M时钟速度
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_10 |
    		GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, & GPIO_InitStructure);

    MATRIX_DISABLE();
    IO_CLR(_IO_LAT);
    IO_CLR(_IO_CLK);
    IO_CLR(_IO_A  );
    IO_CLR(_IO_B  );
    IO_CLR(_IO_C  );
    IO_CLR(_IO_D  );
    IO_CLR(_IO_DR1);
    IO_CLR(_IO_DB1);
    IO_CLR(_IO_DG1);
    IO_CLR(_IO_DR2);
    IO_CLR(_IO_DB2);
    IO_CLR(_IO_DG2);
}

static void _FrameTimerInit(void)
{
    TIM_TimeBaseInitTypeDef timerConfig;
    NVIC_InitTypeDef nvicConfig;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE); //使能TIM1时钟

    nvicConfig.NVIC_IRQChannel = TIM1_UP_IRQn;       		 //选择中断通道
    nvicConfig.NVIC_IRQChannelCmd = ENABLE;              //中断使能
    nvicConfig.NVIC_IRQChannelPreemptionPriority = 1;
    nvicConfig.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvicConfig);

//    TIM_TimeBaseStructInit(&timerConfig);                //初始化TIMBASE结构体
    timerConfig.TIM_ClockDivision = TIM_CKD_DIV1;          // 时钟分频
    // 预分频，计数速度100KHz
    timerConfig.TIM_Prescaler = (SystemCoreClock / 100000u) - 1;
    //  频率： FRAME_FREQ = SystemCoreClock / 4 / （Prescaler + 1） / Period
    timerConfig.TIM_Period = 100000u / _MATRIX_LINE_SCAN_FREQ;
    timerConfig.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数模式
    timerConfig.TIM_RepetitionCounter = 0x00;              //发生0+1次update事件产生中断
    TIM_TimeBaseInit(TIM1, &timerConfig);

    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);              // 开启中断
}

void TIM1_UP_IRQHandler(void)
{
	unsigned int column;

    if(TIM_GetITStatus(TIM1, TIM_IT_Update))
    {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
    else
    {
    	return;
    }

    for(column = 0; column < _MATRIX_COLUMN_COUNT; column ++)
    {
    	IO_CLR(_IO_CLK);

    	IO_SET(_IO_DR1);
    	IO_SET(_IO_DG2);

    	IO_SET(_IO_CLK);
    }

    MATRIX_SCAN_ON_LINE(_currentScanLine);
    IO_CLR(_IO_LAT);
    IO_SET(_IO_LAT);


    _currentScanLine += 1;
    if(_currentScanLine >= _MATRIX_LINE_SCAN_COUNT)
    {
    	_currentScanLine = 0;
    }
}

static void _AnimationTask(void const *args)
{

    MATRIX_ENABLE();
    TIM_Cmd(TIM1, ENABLE);

    while(1)
    {
//    	MATRIX_ENABLE();
//    	osDelay(1);
//    	MATRIX_DISABLE();
    	osDelay(100);
    }
}

