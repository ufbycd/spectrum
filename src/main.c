/**********************2015-10-24**************************************
**功能:实现DSP库的FFT 驱动16X32 RGB彩屏点阵进行频谱显示
256点FFT运算
**DIY视界出品   http://59tiaoba.taobao.com
**作者:Fucp

//标准08接口屏 定义
GND     A     ABCD行扫描
GND     B
GND     C
OE      D	  OE 使能端 
R1      G1	  上半屏红色、绿色
R2      G2	  下半屏红色  绿色
GND	    STCP	  锁存
GND     SHCP	 时钟移位
*************************************************************/

#include "hal.h"
#include "stm32_dsp.h"
uchar switc=0; //
uchar windw=0; //
uchar Miao_color=1;  //
u16 i;
uchar read_tem_bit=1;

int main(void)
{	
	
	ChipHalInit();			//片内硬件初始化
	ChipOutHalInit();		//片外硬件初始化
	delay_init(72);	
	//GPIO_Write(GPIOB, 0xFFFF); 
    color=MemReadByte(0);//读取设置
	  if(color>5)
		color=0;
	 SDA_RGB=olSet[color].proc; //变色	
	 SDA_RGB1=ollSet[color].procc; //变色
	
	color_top=MemReadByte(0x400);//读取设置
	if(color_top>6)
		color_top=0;
	SDA_RGB_top=olSet[color_top].proc; //变色
	 SDA_RGB1_top=ollSet[color_top].procc; //变色
   // USART1_Putc(0x88);
	IWDG_Enable();			//启动看门狗,自动就会使能内部的40K           
 	LEDinit_OFF();			//清屏
	Key1_ON;
	Key2_ON;
  OE=1;
	OE=1;
	for(;;)
	{	  
	   KEY_scan();		//--------------------------------按键扫描
	 	 KEY_Manage  ();  //--------------------------按键执行
		 IWDG_ReloadCounter();	//喂狗
		
	 /*************************************************/
	   if(adc_over==1)
	   {
			adc_over=0;
			
			cr4_fft_256_stm32(FFT_OUT, FFT_IN, NPT);
			FFT();
			TIM2->CR1|=1<<0;   //使能定时器2
			
		}

	 
	/***********************************************/
	}	
} 
/***********************************************************************************
************************************************************************************
*******************DIY视界出品   http://59tiaoba.taobao.com*************************
************************************************************************************
*************************************************************************************/

