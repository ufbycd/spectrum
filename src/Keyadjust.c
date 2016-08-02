#include <hal.h>

uchar menu = 0;
uchar ADJ;                    //按键变量
uchar e = 0;
uchar Read_RI = 0;                  //比较标记
uchar color = 0;
uchar color_top = 0;
/************************************************************/
void KEY_Manage(void)
{
    if (ADJ == 1)
    {
        ADJ = 0XFF;
        color++;
        if (color > 5)
        {
            color = 0;

        }
        SDA_RGB = olSet[color].proc; //变色
        SDA_RGB1 = ollSet[color].procc; //变色
        MemWriteByte(0, color); //保存设置
    }
    if (ADJ == 2)
    {
        ADJ = 0XFF;
        color_top++;
        if (color_top > 6)
        {
            color_top = 0;

        }
        SDA_RGB_top = olSet[color_top].proc; //变色
        SDA_RGB1_top = ollSet[color_top].procc; //变色
        MemWriteByte(0x400, color_top); //保存设置
    }
}
/*********************************************************************
 * 函 数 名: ManageButton
 * 功能描述: 处理按下的按键应该执行的动作
 * 函数说明: 处理按下的按键应该执行的动作
 * 设 计 者 日期：2008-09-11 17:08
 * 修 改 者 日期：2008-09-11 17:08
 * 版 本： v1.0.0
 ***********************************************************************/
/****************** 按键检测与执行 ****************/
uchar ManageButton(uchar Vale)
{
    //遥控学习时不进行按键操控
    if ((GET_LEFT() == 0) || (GET_DOWN() == 0))         //当Ka不为0
    {
        Read_RI = 0;
        if (GET_LEFT() == 0)
        {
            Vale = 1;
        }
        if (GET_DOWN() == 0)
        {
            Vale = 2;
        }
        while ((GET_LEFT() == 0) || (GET_DOWN() == 0))
        {

        }
    }
    return (Vale);
}
/******************************************************************************************/
void KEY_scan(void)
{
    uchar LastKEY = 0;
    LastKEY = ManageButton(LastKEY);
    if (LastKEY != 0)
    {
        ADJ = LastKEY;
        LastKEY = 0;
    }

///////////按键操作/////////////////////////////////////////////////////////////////

}
