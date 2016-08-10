/*
 * audio.c
 *
 *  Created on: 2016年8月3日
 *      Author: chen
 */

#include "audio.h"
#include "stm32_dsp.h"
#include "utils.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>


// 帧频率发生是否用硬件定时器
// 否则用软件定时器
// XXX 此功能目前不稳定
#define HARD_WARE_TIMER_FOR_FRAME 0

// 采样频率（Hz）
#define SAMPLE_FREQ 40000u

// 帧频率（Hz）
#define FRAME_FREQ  25u
// 帧周期时长（ms）
#define FRAME_T		 (1000u / FRAME_FREQ)

// ADC的有效位数
#define ADC_BITS    12

// 运放输出（ADC输入）信号包含vcc/2的直流移置
#define DC_OFFSET  ((1 << ADC_BITS) / 2)

#define FFT_POINTS          256u
#define DMA_BUFFER_lEN     FFT_POINTS
static int16_t _dmaBuf[DMA_BUFFER_lEN];

#define _EVENT_SAMPLE_FINISH 0x1
#define _EVENT_FRAME_BEGIN   (0x1 << 1)
#define _EVENT_FFT_IN_FILL   (0x1 << 2)



typedef union _complex
{
	long iTotal;
	unsigned uTotal;
	struct
	{
		int16_t real;
		int16_t imaginary;
	};
} complex_t;


static osThreadId _sampleTid;
static osThreadId _processTid;
static Util_ResourceHandle_t _fftInResource;

static void _AdcInit(void);
static void _AdcGpioInit(void);
static void _AdcDmaInit(void);
static void _AdcTimerInit(void);
#if HARD_WARE_TIMER_FOR_FRAME
static void _FrameTimerInit(void);
static inline void _FrameTimerStart(void);
#endif
static void _SampleTask(void const *args);
static void _DataProcessTask(void const *args);

void Audio_Init(void)
{
	configASSERT(sizeof(complex_t) == sizeof(long));
	_fftInResource = Util_ResourceCreate(FFT_POINTS * sizeof(complex_t));

    _AdcGpioInit();
    _AdcDmaInit();             //
    _AdcInit();               // 注意此处的初始化顺序，否则采样传输的数据容易出现数据错位的结果
    _AdcTimerInit();           //
#if HARD_WARE_TIMER_FOR_FRAME
    _FrameTimerInit();
#endif

    osThreadDef(audio, _SampleTask, osPriorityRealtime, 0, 128);
    _sampleTid = osThreadCreate(osThread(audio), NULL);

    osThreadDef(process, _DataProcessTask, osPriorityNormal, 0, 512);
    _processTid = osThreadCreate(osThread(process), NULL);
}

static void _AdcInit(void)
{
    ADC_InitTypeDef adcConfig;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 72MHz / 6 = 12MHz ADC时钟速度

    ADC_DeInit(ADC1);           //复位ADC
    ADC_StructInit(&adcConfig);
    //外部触发设置为TIM3
    adcConfig.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
    ADC_Init(ADC1, &adcConfig);
    // 4000KHz采样率要求的一笔采样的时间最多为 1000000us / 4000KHz = 25us
    // 则一笔ADC最大的采样周期数为 25us / (1000000us / 12MHz) - 12.5 = 287.5 (cycle)
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_71Cycles5);
    ADC_Cmd(ADC1, ENABLE);

    /* Enable ADC1 reset calibaration register */
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while (ADC_GetResetCalibrationStatus(ADC1));
    /* Start ADC1 calibration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while (ADC_GetCalibrationStatus(ADC1));

    ADC_DMACmd(ADC1, ENABLE);       //使能ADC_DMA
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

static void _AdcGpioInit(void)
{
    GPIO_InitTypeDef gpioConfig;

    gpioConfig.GPIO_Pin = GPIO_Pin_2;
    gpioConfig.GPIO_Mode = GPIO_Mode_AIN;   // 模拟输入
    GPIO_Init(GPIOA, &gpioConfig);
}

static void _AdcDmaInit(void)
{
    DMA_InitTypeDef dmaConfig;
    NVIC_InitTypeDef nvicConfig;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);     //使能DMA时钟

    nvicConfig.NVIC_IRQChannel = DMA1_Channel1_IRQn;       //选择DMA1通道中断
    nvicConfig.NVIC_IRQChannelCmd = ENABLE;                //中断使能
    // 中断内调用FreeRTOS函数，则需大于或等于此值
    nvicConfig.NVIC_IRQChannelPreemptionPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY;
    nvicConfig.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvicConfig);

    DMA_StructInit(&dmaConfig);                            //初始化DMA结构体
    dmaConfig.DMA_BufferSize = DMA_BUFFER_lEN;            //DMA缓存数组大小设置
    dmaConfig.DMA_DIR = DMA_DIR_PeripheralSRC;             //DMA方向：外设作为数据源
    dmaConfig.DMA_M2M = DISABLE;                           //内存到内存禁用
    dmaConfig.DMA_MemoryBaseAddr = (uint32_t)& _dmaBuf[0];//缓存数据数组起始地址
    dmaConfig.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;//数据大小设置为Halfword
    dmaConfig.DMA_MemoryInc = DMA_MemoryInc_Enable;        //内存地址递增
    dmaConfig.DMA_Mode = DMA_Mode_Circular;                //DMA循环模式，即完成后重新开始覆盖
    dmaConfig.DMA_PeripheralBaseAddr = (uint32_t) &(ADC1->DR);//取值的外设地址
    dmaConfig.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;//外设取值大小设置为Halfword
    dmaConfig.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//外设地址递增禁用
    dmaConfig.DMA_Priority = DMA_Priority_High;             //DMA优先级设置为高
    DMA_Init(DMA1_Channel1, &dmaConfig);

    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); //使能DMA中断
    DMA_ClearITPendingBit(DMA_IT_TC);               //清除一次DMA中断标志
    DMA_Cmd(DMA1_Channel1, ENABLE);                 //使能DMA1
}

static void _AdcTimerInit(void)
{
    TIM_TimeBaseInitTypeDef timerConfig;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);   //使能TIM3时钟

//    TIM_TimeBaseStructInit(&timerConfig);                //初始化TIMBASE结构体
    timerConfig.TIM_ClockDivision = TIM_CKD_DIV1;          //系统时钟，不分频
    // 预分频，计数速度1000KHz
    timerConfig.TIM_Prescaler = (SystemCoreClock / 1000000u) - 1;
    //  采样频率： SAMPLE_FREQ = SystemCoreClock / （Prescaler + 1） / Period
    timerConfig.TIM_Period = 1000000u / SAMPLE_FREQ;
    timerConfig.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数模式
    timerConfig.TIM_RepetitionCounter = 0x00;              //发生0+1次update事件产生中断
    TIM_TimeBaseInit(TIM3, &timerConfig);

    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);   //选择TIM3的update事件更新为触发源
}

#if HARD_WARE_TIMER_FOR_FRAME
static void _FrameTimerInit(void)
{
    TIM_TimeBaseInitTypeDef timerConfig;
    NVIC_InitTypeDef nvicConfig;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //使能TIM4时钟

    nvicConfig.NVIC_IRQChannel = TIM4_IRQn;       		 //选择中断通道
    nvicConfig.NVIC_IRQChannelCmd = ENABLE;              //中断使能
    // 中断内调用FreeRTOS函数，则需大于或等于此值
    nvicConfig.NVIC_IRQChannelPreemptionPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY;
    nvicConfig.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvicConfig);

//    TIM_TimeBaseStructInit(&timerConfig);                //初始化TIMBASE结构体
    timerConfig.TIM_ClockDivision = TIM_CKD_DIV1;          // 时钟分频
    // 预分频，计数速度100KHz
    timerConfig.TIM_Prescaler = (SystemCoreClock / 100000u) - 1;
    //  频率： FRAME_FREQ = SystemCoreClock / 4 / （Prescaler + 1） / Period
    timerConfig.TIM_Period = 100000u / FRAME_FREQ;
    timerConfig.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数模式
    timerConfig.TIM_RepetitionCounter = 0x00;              //发生0+1次update事件产生中断
    TIM_TimeBaseInit(TIM4, &timerConfig);

    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);              // 开启中断
}

static inline void _FrameTimerStart(void)
{
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    TIM_Cmd(TIM4, ENABLE);
}

static inline void _FrameTimerStop(void)
{
	TIM_Cmd(TIM4, DISABLE);
}
#endif

inline static void _SampleStart(void)
{
//	memset(_dmaBuf, 0, sizeof(_dmaBuf));

	TIM_Cmd(TIM3, ENABLE);
    DMA_ClearITPendingBit(DMA_IT_TC);               //清除一次DMA中断标志
    DMA_Cmd(DMA1_Channel1, ENABLE);                 //使能DMA1
    ADC_Cmd(ADC1, ENABLE);
}

inline static void _SampleStop(void)
{
	ADC_Cmd(ADC1, DISABLE);
	TIM_Cmd(TIM3, DISABLE);
	DMA_Cmd(DMA1_Channel1, DISABLE);
}

#if HARD_WARE_TIMER_FOR_FRAME
void TIM4_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM4, TIM_IT_Update))
    {
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        osSignalSet(_sampleTid, _EVENT_FRAME_BEGIN);
    }
}
#endif

void TIM3_IRQHandler(void)
{
    if(TIM_GetITStatus(TIM3, TIM_IT_Update))            //判断发生update事件中断
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);     //清除update事件中断标志
    }
}

void DMA1_Channel1_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA_IT_TC))                      //判断DMA传输完成中断
    {
    	DMA_ClearITPendingBit(DMA_IT_TC);           //清除DMA中断标志位
    	_SampleStop();
    	osSignalSet(_sampleTid, _EVENT_SAMPLE_FINISH);
    }
}

int32_t _CalcDmaBufAverage(complex_t *buf)
{
	unsigned  i;
	int32_t sum;

	sum = 0;
	for(i = 1; i < 21; i++)
	{
		sum += buf[i].real;
	}

	return sum / 20;
}

#if ! HARD_WARE_TIMER_FOR_FRAME
static void _OnFrameBegin(void const *args)
{
	osSignalSet(_sampleTid, _EVENT_FRAME_BEGIN);
}
#endif

static void _SampleTask(void const *args)
{
	osEvent event;
	complex_t *fftInBuf;

#if ! HARD_WARE_TIMER_FOR_FRAME
	osTimerId timid;

	osTimerDef(frameTimer, _OnFrameBegin);
	timid = osTimerCreate(osTimer(frameTimer), osTimerPeriodic, NULL);
	osTimerStart(timid, FRAME_T);
#else
	_FrameTimerStart();
#endif

	while(1)
	{
		event = osSignalWait(_EVENT_FRAME_BEGIN, osWaitForever);
		configASSERT(event.status == osEventSignal);
		_SampleStart();

		event = osSignalWait(_EVENT_SAMPLE_FINISH, osWaitForever);
		if(event.status != osEventSignal)
		{
			configASSERT(false);
			_SampleStop();
			continue;
		}

		fftInBuf = Util_ResourceGet(_fftInResource, osWaitForever);
		if(fftInBuf)
		{
			unsigned i;

			for(i = 0; i < FFT_POINTS; i++)
			{
				fftInBuf[i].real = _dmaBuf[i] - DC_OFFSET;
				fftInBuf[i].imaginary = 0;
			}

			Util_ResourceRelease(_fftInResource);
			osSignalSet(_processTid, _EVENT_FFT_IN_FILL);
		}
		else
		{
			Util_ResourceRelease(_fftInResource);
		}
	}
}

// A率加权曲线，频率间隔：20000Hz / 64 = 312.5Hz，权值为分贝值
// 注：人耳对不频率的声音的敏感度不同，为此引入A率加权来对不同频率进行加权，以符合人的主观感觉
static const float _aRateWeighteds[] =
{
		-6.71, -1.95, -0.21, 0.58,  0.96,  1.16,  1.25,  1.27,  1.25,  1.21,
		1.14,  1.05,  0.94,  0.82,  0.7,   0.56,  0.41,  0.25,  0.09,  -0.09,
		-0.26, -0.45, -0.63, -0.83, -1.02, -1.23, -1.43, -1.64, -1.85, -2.06,
		-2.27, -2.49, -2.71, -2.92, -3.14, -3.36, -3.58, -3.81, -4.03, -4.25,
		-4.47, -4.69, -4.91, -5.13, -5.35, -5.57, -5.79, -6.01, -6.23, -6.44,
		-6.66, -6.87, -7.08, -7.3,  -7.51, -7.72, -7.92, -8.13, -8.34, -8.54,
		-8.74, -8.94, -9.14, -9.34
};

static const float _quantFactors[] =
{
	50, 30, 24, 22, 20, 15, 10, 8, 8, 8,
	8,  8,  8,  8,  8,  8,  8,  8, 8, 8
};

static void _DataProcessTask(void const *args)
{
	osEvent event;
	complex_t *fftInBuf;
	complex_t *fftOutBuf, *pc;
	float  *powerBuf, power, quan;

	fftOutBuf = malloc(FFT_POINTS * sizeof(complex_t));
	configASSERT(fftOutBuf);

	powerBuf = malloc(64 * sizeof(float));
	configASSERT(powerBuf);

	while(1)
	{
		event = osSignalWait(_EVENT_FFT_IN_FILL, osWaitForever);
		if(event.status != osEventSignal)
		{
			continue;
		}

		fftInBuf = Util_ResourceGet(_fftInResource, osWaitForever);
		if(fftInBuf != NULL)
		{
			int i, j;

			cr4_fft_256_stm32(fftOutBuf, fftInBuf, FFT_POINTS);
			Util_ResourceRelease(_fftInResource);

			for(i = 0, j = 1; i < 64; i++, j += 2)
			{
#if 1
				pc = & fftOutBuf[j];
				power = pc->real * pc->real + pc->imaginary * pc->imaginary;
				pc = & fftOutBuf[j + 1];
				power += pc->real * pc->real + pc->imaginary * pc->imaginary;
				power /= 2;
#else
				pc = & fftOutBuf[j];
				power = pc->real * pc->real + pc->imaginary * pc->imaginary;
#endif
#if 0
				powerBuf[i] = power;
				if((i >= 10) && (i < 20))
					printf("%d, ", (int)power);
#else
				// 量化
				if(i < 10)
				{
					quan = _quantFactors[i] * 10;
				}
				else
				{
					quan = 10 - i * 0.140625; // 按不同频率的量化因子不同来处理
				}
				power /= quan;

				// 计算分贝值
				if(power < 1.0001)
				{
					powerBuf[i] = 0;
				}
				else
				{
					powerBuf[i] = 20 * log10f(power) + _aRateWeighteds[i];
				}
#endif
			}
//			printf("\n");

			printf("%.2f %.2f %.2f\n",
					(double)(powerBuf[0]),
					(double)(powerBuf[30]),
					(double)(powerBuf[60]));
		}
		else
		{
			Util_ResourceRelease(_fftInResource);
		}

	}
}


