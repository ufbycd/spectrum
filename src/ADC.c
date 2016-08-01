/**************************************************************
 ADC PB1_ADC9 
 
***************************************************************/

#include "hal.h"
#define ADC1_DR_Address ((u32)0x4001244C)

u16 ADC_ConvertedValue;
void ADC_DMA_Configuration(void)
{
	
	DMA_InitTypeDef DMA_InitStructure;
	/* Enable DMA1 clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	//(2) 初始化 DMA 和 ADC 模块与应用程序
	/* DMA channel1 configuration ———————————————-*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;  // 外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&ADC_ConvertedValue; // 内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;  // DMA 传输方向单向
	DMA_InitStructure.DMA_BufferSize = 1;   // 设置DMA在传输时缓冲区的长度 word
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //设置DMA外设递增模式
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable; // 设置DMA内存递增模式
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  // 外设数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //内存数据字长
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  // 设置传输模式连续不断的循环模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; // 设置DMA的优先级别
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable; // 设置DMA的2个memory中的变量互相访问
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	  /* Enable DMA1 channel1 */
    DMA_Cmd(DMA1_Channel1, ENABLE);

}

void ADC_Configuration(void)
{
	ADC_InitTypeDef   ADC_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);//12M ADC速度
	/* PA2,  ADC采集信号*/
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
		/* ADC1 */
		ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;		//独立模式
		ADC_InitStructure.ADC_ScanConvMode       = DISABLE;						//连续多通道模式
		ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;						//连续转换
		ADC_InitStructure.ADC_ExternalTrigConv   =ADC_ExternalTrigConv_None;	//转换不受外界决定 ADC_ExternalTrigConv_T2_CC2  ;//
		ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;			//右对齐
		ADC_InitStructure.ADC_NbrOfChannel       = 1;							//扫描通道数1
		ADC_Init(ADC1, &ADC_InitStructure);
	
		
	 
		 /* ADC1 regular channels configuration [规则模式通道配置]*/ 
	    ADC_RegularChannelConfig(ADC1, ADC_Channel_2 , 1, ADC_SampleTime_55Cycles5);	//通道X,采用时间为239.5周期,1代表规则通道第1个
	   // ADC_RegularChannelConfig(ADC1, ADC_Channel_9 , 2, ADC_SampleTime_239Cycles5);
    
                        
	 /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, DISABLE);

	ADC_Cmd(ADC1, ENABLE);      /* Enable ADC1*/ 

	  /* Enable ADC1 reset calibaration register */   
  ADC_ResetCalibration(ADC1);
  /* Check the end of ADC1 reset calibration register */
  while(ADC_GetResetCalibrationStatus(ADC1));

  /* Start ADC1 calibration */
  ADC_StartCalibration(ADC1);
  /* Check the end of ADC1 calibration */
  while(ADC_GetCalibrationStatus(ADC1));
     
  /* Start ADC1 Software Conversion */                       
  ADC_SoftwareStartConvCmd(ADC1, ENABLE); // Start ADC1 Software Conversion   使能转换开始

}

u16 TestAdc(void)
{
	u16 adc = 0;
	if(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==SET)
	{
		ADC_ClearFlag(ADC1, ADC_FLAG_EOC); 
		adc=(ADC1->DR);//ADC_GetConversionValue(ADC1);
		//USART1_Putc(ADC_GetConversionValue(ADC1)>>8);
		//USART1_Putc(ADC_GetConversionValue(ADC1));
		//USART1_Puts("\r\n");
	}
	return adc;
}
/***********************************************************************************
************************************************************************************
*******************DIY视界出品   http://59tiaoba.taobao.com*************************
************************************************************************************
*************************************************************************************/
