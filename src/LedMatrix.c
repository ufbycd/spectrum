/*
 * LedMatrix.c
 *
 *  Created on: 2016年8月3日
 *      Author: chen
 */


#include "LedMatrix.h"
#include "audio.h"


#define _MATRIX_COLUMN_COUNT 64
#define _MATRIX_ROW_COUNT    32
#define _MATRIX_COLOR_COUNT  3

// 显示时所需扫描的行数（上下两屏同时扫描）
#define _MATRIX_LINE_SCAN_COUNT (_MATRIX_ROW_COUNT / 2)

// 显示帧频率 Hz
#define _MATRIX_FRAME_FREQ 200

// 行扫描信号频率 Hz
#define _MATRIX_LINE_SCAN_FREQ (_MATRIX_FRAME_FREQ * _MATRIX_LINE_SCAN_COUNT)

// OE PWM信号频率 Hz
#define _MATRIX_RBG_PWM_FREQ (_MATRIX_LINE_SCAN_FREQ * 2)

#define _ADDR_PORT GPIOB
#define _DATA_PORT GPIOB

#define _IO_EN   (GPIOB, GPIO_Pin_11)
#define _IO_LAT  (GPIOA, GPIO_Pin_5)
#define _IO_CLK  (GPIOA, GPIO_Pin_6)
#define _IO_A    (_ADDR_PORT, GPIO_Pin_12)
#define _IO_B    (_ADDR_PORT, GPIO_Pin_13)
#define _IO_C    (_ADDR_PORT, GPIO_Pin_14)
#define _IO_D    (_ADDR_PORT, GPIO_Pin_15)
#define _IO_DR1  (_DATA_PORT, GPIO_Pin_3)
#define _IO_DB1  (_DATA_PORT, GPIO_Pin_4)
#define _IO_DG1  (_DATA_PORT, GPIO_Pin_5)
#define _IO_DR2  (_DATA_PORT, GPIO_Pin_6)
#define _IO_DB2  (_DATA_PORT, GPIO_Pin_7)
#define _IO_DG2  (_DATA_PORT, GPIO_Pin_8)

#define _PORT(p, n) p
#define _PIN(p, n)  n

#define PORT(io) (_PORT io)
#define PIN(io)  (_PIN io)
#define IO_SET(io) PORT(io)->BSRR = PIN(io)
#define IO_CLR(io) PORT(io)->BRR = PIN(io)

#define _SET_D1_COLOR(color) BITS_SET(_DATA_PORT->ODR, 0x7 << 3, (color) << 3)
#define _SET_D2_COLOR(color) BITS_SET(_DATA_PORT->ODR, 0x7 << 6, (color) << 6)

#define _COLOR_D1D2(c1, c2) (c1 | (c2 << 3))
#define _SET_D1D2_COLOR(c1, c2) BITS_SET(_DATA_PORT->ODR, 0x3f << 3, _COLOR_D1D2(c1, c2) << 3)


typedef uint8_t color_t;

#define COLOR_BLACK (0x0)
#define COLOR_RED   (0x1)
#define COLOR_BLUE  (0x1 << 1)
#define COLOR_GREEN (0x1 << 2)

#define MATRIX_ENABLE()  IO_CLR(_IO_EN)
#define MATRIX_DISABLE() IO_SET(_IO_EN)

#define MATRIX_SET_SCAN_LINE(n) BITS_SET(_ADDR_PORT->ODR, 0xf << 12, n << 12)

static volatile unsigned int _currentScanLine;
static volatile uint8_t _spectrumLevels[_MATRIX_COLUMN_COUNT];


static struct
{
	color_t base;
	color_t top;
}_spectrumColor;


static void _GpioInit(void);
static void _FrameTimerInit(void);
static void _PwmGpioInit(void);
static void _PwmTimerInit(void);

static void _AnimationTask(void const *args);
static void _AutoGainControllTask(const void *args);

static osThreadId _animationTid;
static osThreadId _agcTid;

void LedMatrix_Init(void)
{
	_currentScanLine = 0;

	_GpioInit();
	_FrameTimerInit();
	_PwmGpioInit();
	_PwmTimerInit();

    osThreadDef(audio, _AnimationTask, osPriorityRealtime, 0, 128);
    _animationTid = osThreadCreate(osThread(audio), NULL);

    osThreadDef(agc, _AutoGainControllTask, osPriorityBelowNormal, 0, 128);
    _agcTid = osThreadCreate(osThread(agc), NULL);
}

osThreadId LedMatrix_GetDisplayThreadId(void)
{
	return _animationTid;
}

static void _GpioInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //50M时钟速度
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 |
    		GPIO_Pin_8 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    //推挽
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //50M时钟速度
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
    // 预分频，计数速度1000KHz
    timerConfig.TIM_Prescaler = (SystemCoreClock / 1000000u) - 1;
    //  频率： FRAME_FREQ = SystemCoreClock / （Prescaler + 1） / (Period + 1)
    timerConfig.TIM_Period = (1000000u / _MATRIX_LINE_SCAN_FREQ) - 1;
    timerConfig.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数模式
    timerConfig.TIM_RepetitionCounter = 0x00;              //发生0+1次update事件产生中断
    TIM_TimeBaseInit(TIM1, &timerConfig);

    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);              // 开启中断
}

static void _FrameTimerStart(void)
{
    TIM_Cmd(TIM1, ENABLE);
}

static void _PwmGpioInit(void)
{
    GPIO_InitTypeDef gpioConfig;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    gpioConfig.GPIO_Pin = GPIO_Pin_11;
    gpioConfig.GPIO_Mode = GPIO_Mode_AF_PP;
    gpioConfig.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, & gpioConfig);

    GPIO_PinRemapConfig(GPIO_FullRemap_TIM2, ENABLE);
}

static uint16_t _tim2Period;

static void _PwmTimerInit(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;

    /*PCLK1经过2倍频后作为TIM3的时钟源等于72MHz*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    /* Time base configuration */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;        //设置时钟分频系数：不分频
    TIM_TimeBaseStructure.TIM_Prescaler = (SystemCoreClock / 1000000u) - 1;
    _tim2Period = 1000000u / _MATRIX_RBG_PWM_FREQ;
    TIM_TimeBaseStructure.TIM_Period = _tim2Period - 1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;    //向上计数溢出模式
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;              //配置为PWM模式1

    /* PWM1 Mode configuration: Channel4 */
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = _tim2Period * 10 / 100;        //占比宽度
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;       //占比时为低电平
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);                       //使能通道2
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

    TIM_ARRPreloadConfig(TIM2, ENABLE);                            //使能TIM重载寄存器ARR
}

static void _PwmStart(void)
{
    TIM_Cmd(TIM2, ENABLE);                                         //使能TIM
}

void LedMatrix_SetBrightness(float level)
{
	uint16_t ccr4;

	if(level > 100)
	{
		ccr4 = _tim2Period;
	}
	else if(level < 0)
	{
		ccr4 = 0;
	}
	else
	{
		ccr4 = (uint16_t) (_tim2Period * level / 100);
	}

	TIM2->CCR4 = ccr4;
}

void LedMatrix_SetSpectrumColor(color_t base, color_t top)
{
	_spectrumColor.base = base;
	_spectrumColor.top  = top;
}

void TIM1_UP_IRQHandler(void)
{
	unsigned int column;
	uint8_t level;
	color_t base;

    if(TIM_GetITStatus(TIM1, TIM_IT_Update))
    {
        TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
    }
    else
    {
    	return;
    }

    IO_CLR(_IO_LAT);
    MATRIX_SET_SCAN_LINE(_currentScanLine);

    base = _spectrumColor.base;
    for(column = 0; column < _MATRIX_COLUMN_COUNT; column ++)
    {
    	IO_CLR(_IO_CLK);

        level = _spectrumLevels[column];
    	if(level > (_currentScanLine + 16))
    	{
        	_SET_D1D2_COLOR(base, base);
    	}
    	else if(level > _currentScanLine)
    	{
    		_SET_D1D2_COLOR(base, COLOR_BLACK);
    	}
    	else
    	{
    		_SET_D1D2_COLOR(COLOR_BLACK, COLOR_BLACK);
    	}

    	IO_SET(_IO_CLK);
    }

    IO_SET(_IO_LAT);

    _currentScanLine += 1;
    if(_currentScanLine >= _MATRIX_LINE_SCAN_COUNT)
    {
    	_currentScanLine = 0;
    }
}

static void _Test(void)
{
	int i, level;
	const color_t testColors[] = {COLOR_RED, COLOR_BLUE, COLOR_GREEN, COLOR_BLACK,
	COLOR_RED | COLOR_BLUE, COLOR_RED | COLOR_GREEN, COLOR_BLUE | COLOR_GREEN,
	COLOR_RED | COLOR_BLUE | COLOR_GREEN};

	for(i = 0; i < _MATRIX_COLUMN_COUNT; i++)
	{
		_spectrumLevels[i] = _MATRIX_ROW_COUNT;
	}

	for(i = 0; i < ARRAY_SIZE(testColors); i++)
	{
		LedMatrix_SetSpectrumColor(testColors[i], 0);
		for(level = 0; level < 100 + 1; level += 5)
		{
			LedMatrix_SetBrightness(level);
			osDelay(100);
		}
	}

	for(i = 0; i < _MATRIX_COLUMN_COUNT; i++)
	{
		_spectrumLevels[i] = 0;
	}
}

static void _AnimationTask(void const *args)
{
	float *fspectrumBuf;
	osEvent event;
	int i, level;

	for(i = 0; i < _MATRIX_COLUMN_COUNT; i++)
	{
		_spectrumLevels[i] = 0;
	}

	_FrameTimerStart();
    _PwmStart();

//    _Test();
	LedMatrix_SetBrightness(10);
	LedMatrix_SetSpectrumColor(COLOR_BLUE, COLOR_RED);

    while(1)
    {
    	event = osSignalWait(EVENT_SPECTRUM_FILL, osWaitForever);
		if(event.status != osEventSignal)
		{
			continue;
		}

		fspectrumBuf = Util_ResourceGet(Audio_GetSpectrumResHandle(), osWaitForever);
		if(fspectrumBuf != NULL)
		{
			for(i = 0; i < _MATRIX_COLUMN_COUNT; i++)
			{
				level = (int) fspectrumBuf[i];
				if(level < 0)
				{
					level = 0;
				}
				else if(level > _MATRIX_ROW_COUNT)
				{
					level = _MATRIX_ROW_COUNT;
				}
				_spectrumLevels[i] = (uint8_t) level;
			}

			Util_ResourceRelease(Audio_GetSpectrumResHandle());
		}
    }
}

/**
 * @brief 通过软件实现自动增益控制（AGC），以克服因音源的音量的不同
 *        而造成频谱显示时其饱满度差别过大问题
 * @param args
 */
static void _AutoGainControllTask(const void *args)
{
	osEvent event;


	while(1)
	{
		event = osSignalWait(EVENT_SPECTRUM_FILL, osWaitForever);
		if(event.status != osEventSignal)
		{
			continue;
		}
	}
}
