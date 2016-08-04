/*
 * audio.c
 *
 *  Created on: 2016年8月3日
 *      Author: chen
 */

#include "audio.h"


#define DMA_BUFFER_SIZE     6
int16_t adc_dma_buf[DMA_BUFFER_SIZE];


static void _AdcConfig(void);
static void _AdcGpioInit(void);
static void _AdcDmaInit(void);
static void _AdcTimerInit(void);

void Audio_Init(void)
{
    _AdcGpioInit();
    _AdcConfig();               // 注意此处的初始化顺序，否则采样传输的数据容易出现数据错位的结果
    _AdcDmaInit();             //
    _AdcTimerInit();           //
}

void user_adc_init()
{
    _AdcGpioInit();
    _AdcConfig();               // 注意此处的初始化顺序，否则采样传输的数据容易出现数据错位的结果
    _AdcDmaInit();             //
    _AdcTimerInit();           //

}

static void _AdcConfig(void)
{
    ADC_InitTypeDef adcConfig;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // 72MHz / 6 = 12MHz ADC速度

    ADC_DeInit(ADC1);           //复位ADC
    ADC_StructInit(&adcConfig);
    //外部触发设置为TIM3
    adcConfig.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T3_TRGO;
    ADC_Init(ADC1, &adcConfig);
    //配置采样通道，采样时间125nS
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 1, ADC_SampleTime_55Cycles5);
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
    nvicConfig.NVIC_IRQChannelPreemptionPriority = 0;      //优先级设为0
    NVIC_Init(&nvicConfig);

    DMA_StructInit(&dmaConfig);                            //初始化DMA结构体
    dmaConfig.DMA_BufferSize = DMA_BUFFER_SIZE;            //DMA缓存数组大小设置
    dmaConfig.DMA_DIR = DMA_DIR_PeripheralSRC;             //DMA方向：外设作为数据源
    dmaConfig.DMA_M2M = DISABLE;                           //内存到内存禁用
    dmaConfig.DMA_MemoryBaseAddr = (uint32_t)&adc_dma_buf[0];//缓存数据数组起始地址
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
    NVIC_InitTypeDef nvicConfig;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);   //使能TIM2时钟
    nvicConfig.NVIC_IRQChannel = TIM2_IRQn;                //选择TIM2中断通道
    nvicConfig.NVIC_IRQChannelCmd = ENABLE;                //使能TIM2中断
    nvicConfig.NVIC_IRQChannelPreemptionPriority = 0;      //优先级为0
    NVIC_Init(&nvicConfig);

    TIM_TimeBaseStructInit(&timerConfig);                  //初始化TIMBASE结构体
    timerConfig.TIM_ClockDivision = TIM_CKD_DIV1;          //系统时钟，不分频，48M
    timerConfig.TIM_CounterMode = TIM_CounterMode_Up;      //向上计数模式
    timerConfig.TIM_Period = 312;                          //每312 uS触发一次中断，开启ADC
    timerConfig.TIM_Prescaler = 48-1;                      //计数时钟预分频，f＝1M，systick＝1 uS
    timerConfig.TIM_RepetitionCounter = 0x00;              //发生0+1次update事件产生中断
    TIM_TimeBaseInit(TIM3, &timerConfig);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);              //使能TIM2中断
    TIM_SelectOutputTrigger(TIM3, TIM_TRGOSource_Update);   //选择TIM2的update事件更新为触发源
    TIM_Cmd(TIM3, ENABLE);                                  //使能TIM2
}


void TIM2_IRQHandler(void)
{

}

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
        TIM_Cmd(TIM2, DISABLE);                     //完成周波采样，停止定时器
        DMA_Cmd(DMA1_Channel1, DISABLE);            //完成周波采样，停止DMA
    }
    DMA_ClearITPendingBit(DMA_IT_TC);                   //清除DMA中断标志位
}