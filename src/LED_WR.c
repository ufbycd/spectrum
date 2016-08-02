#include <stdlib.h>
#include "hal.h"

//#include "STM32Lib\\stm32f10x.h"
u8 hh, ss;
u8 week;
u8 x;
/****************上半屏颜色控制********************************/
void SDA_RS(uchar i)
{
    SDA_R1 = (_Bool) i;
}
void SDA_GS(uchar i)
{
    SDA_G1 = (_Bool) i;
}
void SDA_BS(uchar i)
{
    SDA_B1 = (_Bool) (i);
}
void SDA_RGS(uchar i)
{
    SDA_R1 = (_Bool) (i);
    SDA_G1 = (_Bool) (i);
}
void SDA_GBS(uchar i)
{
    SDA_G1 = (_Bool) (i);
    SDA_B1 = (_Bool) (i);
}
void SDA_BRS(uchar i)
{
    SDA_R1 = (_Bool) (i);
    SDA_B1 = (_Bool) (i);
}
void SDA_BGRS(uchar i)
{
    SDA_R1 = (_Bool) (i);
    SDA_G1 = (_Bool) (i);
    SDA_B1 = (_Bool) (i);
}

void (*SDA_RGB_top)(uchar);
void (*SDA_RGB)(uchar);
//typedef struct bb
//{
//    void (*proc)(uchar);
//}_ol_;
_ol_ olSet[7] =
{
{ SDA_RS },
{ SDA_GS },
{ SDA_BS },
{ SDA_RGS },
{ SDA_GBS },
{ SDA_BRS },
{ SDA_BGRS },

};
/*******************下半屏颜色控制************************/
void SDA_R(uchar i)
{
    SDA_R2 = (_Bool) i;
}
void SDA_G(uchar i)
{
    SDA_G2 = (_Bool) i;
}
void SDA_B(uchar i)
{
    SDA_B2 = (_Bool) i;
}
void SDA_RG(uchar i)
{
    SDA_R2 = (_Bool) i;
    SDA_G2 = (_Bool) i;
}
void SDA_GB(uchar i)
{
    SDA_G2 = (_Bool) i;
    SDA_B2 = (_Bool) i;
}
void SDA_BR(uchar i)
{
    SDA_R2 = (_Bool) i;
    SDA_B2 = (_Bool) i;
}
void SDA_BGR(uchar i)
{
    SDA_R2 = (_Bool) i;
    SDA_G2 = (_Bool) i;
    SDA_B2 = (_Bool) i;
}
void (*SDA_RGB1_top)(uchar);
void (*SDA_RGB1)(uchar);
//typedef struct b
//{
//    void (*procc)(uchar);
//}_oll_;
_oll_ ollSet[7] =
{
{ SDA_R },
{ SDA_G },
{ SDA_B },
{ SDA_RG },
{ SDA_GB },
{ SDA_BR },
{ SDA_BGR },

};
/**************************************************/
void LEDinit_OFF(void) //清屏
{
    uchar x;
    for (x = 0; x < 64; x++) //把R,G缓冲区全部数据清0
    {
        LED_TAB[x] = 0;
        LED_TAB1[x] = 0;
    }
}

/**************************************************************/

void delay_5us(uchar time)   //10us
{
    unsigned char a, b, c;
    for (c = time; c > 0; c--)
    {
        for (b = 30; b > 0; b--)
            for (a = 5; a > 0; a--)
                ;
    }
}
/***************************************/
void scan(unsigned char Value) //行扫描
{
    switch (Value)
    {
    case 0:
        scan0
        ;
        break;
    case 1:
        scan1
        ;
        break;
    case 2:
        scan2
        ;
        break;
    case 3:
        scan3
        ;
        break;
    case 4:
        scan4
        ;
        break;
    case 5:
        scan5
        ;
        break;
    case 6:
        scan6
        ;
        break;
    case 7:
        scan7
        ;
        break;
    case 8:
        scan8
        ;
        break;
    case 9:
        scan9
        ;
        break;
    case 10:
        scan10
        ;
        break;
    case 11:
        scan11
        ;
        break;
    case 12:
        scan12
        ;
        break;
    case 13:
        scan13
        ;
        break;
    case 14:
        scan14
        ;
        break;
    case 15:
        scan15
        ;
        break;
    default:
        break;
    }
}
/*******************************************************************************
 * Function Name  : TIM3_IRQHandler
 * Description    : This function handles TIM3 global interrupt request.
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void TIM3_IRQHandler(void)
{

    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);

        /**************************************************************/
        OE = 1;
        OE1 = 1;
        for (G = 0; G < 32; G++)						  //往点阵屏填充 一行的 数据
        {

            SDA_G1 = SDA_B1 = SDA_R1 = 0;
            SDA_G2 = SDA_B2 = SDA_R2 = 0;
            if (LED_TAB[G] > LINE)
            {
                (*SDA_RGB1)(1); //发送对应 R G B信号
            }
            if (LED_TAB[G] > (LINE + 16))
            {
                (*SDA_RGB)(1); //发送对应 R G B信号

            }
            if (LED_TAB1[G] == LINE)
            {
                (*SDA_RGB1_top)(1); //发送对应 R G B信号

            }
            if (LED_TAB1[G] == (LINE + 16))
            {
                (*SDA_RGB_top)(1); //发送对应 R G B信号

            }
            SHCP = 1;
            SHCP = 0;
            SHCP1 = 1;
            SHCP1 = 0;
            SHCP = 1;
            SHCP = 0;
            SHCP1 = 1;
            SHCP1 = 0;

        }
        STCP = 1;
        STCP = 0;
        STCP1 = 1;
        STCP1 = 0;
        scan(15 - LINE);
        if (LINE > 0)
            LINE--;
        else
            LINE = 15;

        if (++ss > 2)
        {
            ss = 0;
            for (x = 0; x < 1; x++) //根据是16分频 与32分频 来判断柱状递减的速度
            {

                if (LED_TAB[COUNT] > 0)
                    LED_TAB[COUNT]--;		   //柱状递减，

                if (COUNT > 0)
                    COUNT--;
                else
                {

                    COUNT = 31;

                }

            }
        }
        //	if(fractional_frequency<33)   //判断是否是16分频
        //{
        week++;
        if (week == 3)
        {
            week = 0;
            hh = 1;
        }
        else
            hh = 0;
        //	}

        for (x = 0; x < hh; x++)	//16  Stop_time 变量漂浮点递减速度控制
        {
            if (LED_TAB2[COUNT1] == 0)	//漂浮点是否停留
            {
                if (LED_TAB1[COUNT1] > LED_TAB[COUNT1])  //漂浮点>柱状 吗
                    LED_TAB1[COUNT1]--;
            }
            else
                LED_TAB2[COUNT1]--;
            COUNT1++;
        }
        if (COUNT1 >= 32)
            COUNT1 = 0;
        OE = 0;
        OE1 = 0;
    }

    /*************************************************/
}
/***********************************************************************************
 ************************************************************************************
 *******************DIY视界出品   http://59tiaoba.taobao.com*************************
 ************************************************************************************
 *************************************************************************************/

