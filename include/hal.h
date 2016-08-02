#ifndef HAL_H
#define HAL_H

#include <stdbool.h>
#include "stm32f10x.h"
#include "delay.h"

#define uchar unsigned char
#define uint unsigned int
#define NPT 256            /* NPT = No of FFT point*/
//硬件初始化
extern void  ChipHalInit(void);
extern void  ChipOutHalInit(void);
u16 TestAdc(void);

 //串口
extern void USART1_Putc(u8 c);
extern void USART1_Puts(char * str);
//位带操作,实现51类似的GPIO控制功能
//具体实现思想,参考<<CM3权威指南>>第五章(87页~92页).
//IO口操作宏定义
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr &0xFFFFF)<<5)+(bitnum<<2)) 
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr)) 
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum)) 
//IO口地址映射
#define GPIOA_ODR_Addr    (GPIOA_BASE+12) //0x4001080C 
#define GPIOB_ODR_Addr    (GPIOB_BASE+12) //0x40010C0C 
#define GPIOC_ODR_Addr    (GPIOC_BASE+12) //0x4001100C 
//#define GPIOD_ODR_Addr    (GPIOD_BASE+12) //0x4001140C 
//#define GPIOE_ODR_Addr    (GPIOE_BASE+12) //0x4001180C 
//#define GPIOF_ODR_Addr    (GPIOF_BASE+12) //0x40011A0C    
//#define GPIOG_ODR_Addr    (GPIOG_BASE+12) //0x40011E0C    

#define GPIOA_IDR_Addr    (GPIOA_BASE+8) //0x40010808 
#define GPIOB_IDR_Addr    (GPIOB_BASE+8) //0x40010C08 
#define GPIOC_IDR_Addr    (GPIOC_BASE+8) //0x40011008 
//#define GPIOD_IDR_Addr    (GPIOD_BASE+8) //0x40011408 
//#define GPIOE_IDR_Addr    (GPIOE_BASE+8) //0x40011808 
//#define GPIOF_IDR_Addr    (GPIOF_BASE+8) //0x40011A08 
//#define GPIOG_IDR_Addr    (GPIOG_BASE+8) //0x40011E08 
 
//IO口操作,只对单一的IO口!
//确保n的值小于16!
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  //输出 
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  //输入 

#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  //输出 
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  //输入 

#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  //输出 
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  //输入 

#define SHCP PBout(8) //595的移位信号          
#define STCP PAout(0)//是595的锁存信号     

#define SHCP1 PAout(8) //595的移位信号          
#define STCP1 PBout(15)//是595的锁存信号  

#define SDA_R1  PBout(14)//上半屏红数据  
#define SDA_G1  PBout(13)//上半屏绿数据 
#define SDA_B1  PAout(14)//上半屏蓝数据  

#define SDA_R2 PAout(1)//下半屏红数据  
#define SDA_G2 PAout(3)//下半屏绿数据 
#define SDA_B2 PBout(5)//下半屏蓝数据

#define OE  PAout(4)//输出使能 
#define OE1  PBout(12)//输出使能

#define Key1_ON	GPIOB->BSRR = GPIO_Pin_0
#define Key2_ON	GPIOB->BSRR = GPIO_Pin_1

//#define LED_ON		GPIOC->BSRR = GPIO_Pin_13
//#define LED_OFF	    GPIOC->BRR = GPIO_Pin_13
//输出宏定义
#define LA_ON	   GPIOA->BSRR = GPIO_Pin_5
#define LA_OFF   GPIOA->BRR = GPIO_Pin_5

#define LB_ON	   GPIOA->BSRR = GPIO_Pin_6
#define LB_OFF   GPIOA->BRR = GPIO_Pin_6

#define LC_ON	   GPIOA->BSRR = GPIO_Pin_7
#define LC_OFF   GPIOA->BRR = GPIO_Pin_7

#define LD_ON	   GPIOC->BSRR = GPIO_Pin_13
#define LD_OFF   GPIOC->BRR = GPIO_Pin_13
/***********************************************************/
#define LA1_ON	  GPIOB->BSRR = GPIO_Pin_11
#define LA1_OFF   GPIOB->BRR = GPIO_Pin_11

#define LB1_ON	  GPIOB->BSRR = GPIO_Pin_10
#define LB1_OFF   GPIOB->BRR = GPIO_Pin_10

#define LC1_ON	  GPIOB->BSRR = GPIO_Pin_9
#define LC1_OFF   GPIOB->BRR = GPIO_Pin_9

#define LD1_ON	  GPIOA->BSRR = GPIO_Pin_13
#define LD1_OFF   GPIOA->BRR = GPIO_Pin_13

#define  scan0    {LA_OFF;LB_OFF;LC_OFF;LD_OFF;LA1_OFF;LB1_OFF;LC1_OFF;LD1_OFF;}
#define  scan1    {LA_ON; LB_OFF;LC_OFF;LD_OFF;LA1_ON; LB1_OFF;LC1_OFF;LD1_OFF;}
#define  scan2    {LA_OFF;LB_ON; LC_OFF;LD_OFF;LA1_OFF;LB1_ON; LC1_OFF;LD1_OFF;}
#define  scan3    {LA_ON;LB_ON;LC_OFF;LD_OFF;LA1_ON;LB1_ON;LC1_OFF;LD1_OFF;}
#define  scan4    {LA_OFF;LB_OFF;LC_ON;LD_OFF;LA1_OFF;LB1_OFF;LC1_ON;LD1_OFF;}
#define  scan5    {LA_ON;LB_OFF;LC_ON;LD_OFF;LA1_ON;LB1_OFF;LC1_ON;LD1_OFF;}
#define  scan6    {LA_OFF;LB_ON;LC_ON;LD_OFF;LA1_OFF;LB1_ON;LC1_ON;LD1_OFF;}
#define  scan7    {LA_ON;LB_ON;LC_ON;LD_OFF;LA1_ON;LB1_ON;LC1_ON;LD1_OFF;}
#define  scan8    {LA_OFF;LB_OFF;LC_OFF;LD_ON;LA1_OFF;LB1_OFF;LC1_OFF;LD1_ON;}

#define  scan9    {LA_ON;LB_OFF;LC_OFF;LD_ON;LA1_ON;LB1_OFF;LC1_OFF;LD1_ON;}
#define scan10    {LA_OFF;LB_ON;LC_OFF;LD_ON;LA1_OFF;LB1_ON;LC1_OFF;LD1_ON;}
#define scan11    {LA_ON;LB_ON;LC_OFF;LD_ON;LA1_ON;LB1_ON;LC1_OFF;LD1_ON;}
#define scan12    {LA_OFF;LB_OFF;LC_ON;LD_ON;LA1_OFF;LB1_OFF;LC1_ON;LD1_ON;}
#define scan13    {LA_ON;LB_OFF;LC_ON;LD_ON;LA1_ON;LB1_OFF;LC1_ON;LD1_ON;}
#define scan14    {LA_OFF;LB_ON;LC_ON;LD_ON;LA1_OFF;LB1_ON;LC1_ON;LD1_ON;}
#define scan15    {LA_ON;LB_ON;LC_ON;LD_ON;LA1_ON;LB1_ON;LC1_ON;LD1_ON;}
/*****??????????*************************/
extern void SDA_RS(uchar i) ;
extern void SDA_GS(uchar i) ;
extern void SDA_BS(uchar i);
extern void SDA_RGS(uchar i); 
extern void SDA_GBS(uchar i);
extern void SDA_BRS(uchar i);
extern void SDA_BGRS(uchar i);

extern void (*SDA_RGB_top)(uchar);
extern void (*SDA_RGB)(uchar);
typedef struct b
{
    void (*procc)(uchar);
}_oll_;
extern _oll_ ollSet[7];

extern void SDA_R(uchar i) ;
extern void SDA_G(uchar i) ;
extern void SDA_B(uchar i);
extern void SDA_RG(uchar i); 
extern void SDA_GB(uchar i);
extern void SDA_BR(uchar i);
extern void SDA_BGR(uchar i);

extern void (*SDA_RGB1_top)(uchar);
extern void (*SDA_RGB1)(uchar);
typedef struct bb
{
    void (*proc)(uchar);
}_ol_;
extern _ol_ olSet[7];

//#define LED_ON		GPIOC->BSRR = GPIO_Pin_13
//#define LED_OFF	    GPIOC->BRR = GPIO_Pin_13
//设置DATA


//sbit   DQ= P3^6 ;
//输入宏定义
//#define GET_RIGHT()	(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_11))
//#define GET_UP()	(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_12))
#define GET_DOWN()	(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1))
#define GET_LEFT()	(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_0))
//#define SET_DOWN()	GPIOB->BSRR = GPIO_Pin_6
//#define RSET_DOWN()	GPIOB->BRR = GPIO_Pin_6
extern uchar  LED_TAB2[64];				//记录 漂浮物 是否需要 停顿一下
extern uchar  LED_TAB[64];				//记录红色柱状 
extern uchar  LED_TAB1[64];				 	//记录 漂浮点
extern u16 ADC_ConvertedValue;		
extern void FFT(void);
extern u16 ADC_Count;
extern u16 TestAdc(void);
extern u8 adc_over;
/*****************task.c***************************************/
extern void SendByte(unsigned char dat);
extern uchar switc;
extern uchar windw; 
extern uchar old_ss;
extern uchar Miao_color;
extern uchar read_tem_bit;
extern uchar Light;

/**********************LED_WR.C******************************************************/
extern  void LEDinit(void);//清屏
/***********************Key_adjust*******************************************************/
extern void KEY_Manage(void);
extern void KEY_scan(void);
extern uchar ADJ;                    //按键变量
extern uchar menu;
extern void LEDinit_OFF(void);//清屏
extern void  Backtotime(void);
extern void ADC_Configuration(void);			//ADC
extern uchar color;
extern uchar color_top;

/**************************LED_WR.C********************************************/
//extern uchar switc;
extern void delay(uint xms);
extern uchar Read_adc;
/***************************FFT.C**********************************************/
extern  void Scan_FFT_Mode(void);
extern uchar fractional_frequency;
extern uchar G;
extern uchar Mode;//模式
//extern int  Fft_Real[128]; 
//extern int  Fft_Image[128];               // fft的虚部 
//extern uchar xdata LED_TAB2[64];				//记录 漂浮物 是否需要 停顿一下
//extern uchar xdata LED_TAB[64];				//记录红色柱状 
//extern uchar xdata LED_TAB1[64];				//记录 漂浮点
extern uchar LINE;
extern uchar FFTcolor;
//extern uchar j; 
extern uchar COUNT,COUNT1;
//extern uchar count_time;
//extern void FFT();
extern long FFT_IN[NPT];
extern long FFT_OUT[NPT]; 
/********************Flash.C*****************************************************/
extern u8 MemWriteByte(u16 address,u16 data);
extern u8 MemReadByte(u16 address) ;
#endif
