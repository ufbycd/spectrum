#include "stm32f10x.h"

#include <hal.h>
#include <math.h>
#define STRT 	4	//规则通道开始位
#define JSTRT	3	//注入通道开始位
#define JEOC	2	//注入通道转换结束位
#define EOC		1	//规则通道转换结束位
#define AWD		0	//模拟看门狗标志位
#define	ADON		0	//ADC使能位

uchar COUNT = 15, COUNT1 = 0, LINE = 15, G;
u16 ADC_Count = 0;
u8 FFTcolor;
u8 Mode = 0;
int Temp_Real, Temp_Imag, temp;                // 中间临时变量
uint TEMP1;
//uint p1,p2,p3;
//uint tx;
u8 adc_over = 0;   //256次adc转换完成

uchar fractional_frequency = 64;   //分频
extern uint16_t TableFFT[];
long FFT_IN[NPT]; /* Complex input vector */
long FFT_OUT[NPT]; /* Complex output vector */

u8 LED_TAB2[64];				//记录 漂浮物 是否需要 停顿一下
u8 LED_TAB[64];				//记录红色柱状
u8 LED_TAB1[64];				//记录 漂浮点
/*******************************************/
void FFT(void)
{
    u8 j;
    int32_t lX, lY;
    for (j = 5; j < 37; j++)
    {

        lX = (FFT_OUT[j] << 16) >> 16;
        lY = (FFT_OUT[j] >> 16);
        {
//      float X=  256*((float)lX)/32768;
//      float Y = 256*((float)lY)/32768;
            //float Mag =   // 先平方和,再开方
            TEMP1 = sqrt(lX * lX + lY * lY) / 7;
            if (TEMP1 < 6)
                TEMP1 = 0;
            else
            {
                TEMP1 = TEMP1 - 6;
            }

            /****************************************************************************************************************/

            if (TEMP1 > 31)
                TEMP1 = 31;
            if (TEMP1 > (LED_TAB[j - 5]))
                LED_TAB[j - 5] = TEMP1;
            if (TEMP1 > (LED_TAB1[j - 5]))
            {
                LED_TAB1[j - 5] = TEMP1;
                LED_TAB2[j - 5] = 10;                                                //提顿速度=12
            }
        }

    }
}

/*******************************************************************************
 * Function Name  : TIM2_IRQHandler TIM2中断
 * Description    : This function handles TIM2 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void TIM2_IRQHandler(void)
{
    /**************************************************/
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        //Fft_Real[LIST_TAB[ADC_Count]]=ADC_ConvertedValue;//读取ad结果 //ADC_ConvertedValue[0];
        ADC1->CR2 |= (1 << ADON);			//启动转换
        //ADC1->CR2|=(1<<ADON);			//启动转换
        //TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
        TIM2->SR &= ~(1 << 0);		    //清楚中断标志
        //	while(ADC_ConvertedValue>0XFF0)

        FFT_IN[ADC_Count] = (TestAdc() - 500) << 16;
        if (ADC_Count <= 255)
            ADC_Count++;
        else
        {
            TIM2->CR1 &= ~(1 << 0);   //关闭定时器2
            ADC_Count = 0;
            adc_over = 1;
            //USART1_Putc(ADC_ConvertedValue);
            // TIM_ITConfig( TIM2,TIM_IT_CC4,DISABLE ); //关闭TIM_IT_CC4中断
            // ADC_Cmd(ADC1, DISABLE);
        }

    }
}

