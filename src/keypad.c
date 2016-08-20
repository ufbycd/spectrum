/*
 * keypad.c
 *
 *  Created on: 2016年8月19日
 *      Author: chenss
 */


#include "keypad.h"
#include "led.h"
#include "audio.h"

#define _IO_KEY0 (GPIOB, GPIO_Pin_0)
#define _IO_KEY1 (GPIOB, GPIO_Pin_1)

#define _LOCAL_EVENT_KEY0 (0x1)
#define _LOCAL_EVENT_KEY1 (0x1 << 2)

#define _KEY_DETECT_METHOD_INTERRUPT 0

#define _KEY_PRESSED(io) (GPIO_ReadInputDataBit(PORT(io), PIN(io)) == Bit_RESET)

static osThreadId _keyTid;


#if ! _KEY_DETECT_METHOD_INTERRUPT
#	define Util_NVIC_Cmd(n, e)
#endif

static void _GpioInit(void);
static void _KeyDetectTask(void const *args);

void Keypad_Init(void)
{

	_GpioInit();
//	if(_KEY_PRESSED(_IO_KEY0))
//	{
//		Audio_SetCalibrationOn();
//	}

    osThreadDef(key, _KeyDetectTask, osPriorityLow, 0, 128);
    _keyTid = osThreadCreate(osThread(key), NULL);
}

static void _GpioInit(void)
{
    GPIO_InitTypeDef gpioConfig;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gpioConfig.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    gpioConfig.GPIO_Mode = GPIO_Mode_IPU;
    gpioConfig.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_Init(GPIOB, & gpioConfig);

#if _KEY_DETECT_METHOD_INTERRUPT

    NVIC_InitTypeDef nvicConfig;
    EXTI_InitTypeDef extiConfig;

    nvicConfig.NVIC_IRQChannel = EXTI0_IRQn;
    nvicConfig.NVIC_IRQChannelCmd = ENABLE;                //中断使能
    // 中断内调用FreeRTOS函数，则需大于或等于此值
    nvicConfig.NVIC_IRQChannelPreemptionPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY + 2;
    nvicConfig.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvicConfig);

    nvicConfig.NVIC_IRQChannel = EXTI1_IRQn;
    nvicConfig.NVIC_IRQChannelCmd = ENABLE;                //中断使能
    // 中断内调用FreeRTOS函数，则需大于或等于此值
    nvicConfig.NVIC_IRQChannelPreemptionPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY + 2;
    nvicConfig.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&nvicConfig);

    EXTI_ClearITPendingBit(EXTI_Line0 | EXTI_Line1);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource1);
    extiConfig.EXTI_Line = EXTI_Line0 | EXTI_Line1;
    extiConfig.EXTI_Mode = EXTI_Mode_Interrupt;
    extiConfig.EXTI_Trigger = EXTI_Trigger_Falling;
    extiConfig.EXTI_LineCmd = ENABLE;
    EXTI_Init(& extiConfig);
#endif
}

#if _KEY_DETECT_METHOD_INTERRUPT
void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line0))
	{
		Util_NVIC_Cmd(EXTI0_IRQn, DISABLE);
		osSignalSet(_keyTid, _LOCAL_EVENT_KEY0);

		EXTI_ClearITPendingBit(EXTI_Line0);
	}
}


void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line1))
	{
		Util_NVIC_Cmd(EXTI1_IRQn, DISABLE);
		osSignalSet(_keyTid, _LOCAL_EVENT_KEY1);

		EXTI_ClearITPendingBit(EXTI_Line1);
	}
}
#else

static uint32_t _keyTrigger;
static uint32_t _keyContinue;
static void _KeyScan(void const *args)
{
	uint32_t keyEvent;

	keyEvent = 0;
	if(_KEY_PRESSED(_IO_KEY0))
	{
		keyEvent |= _LOCAL_EVENT_KEY0;
	}

	if(_KEY_PRESSED(_IO_KEY1))
	{
		keyEvent |= _LOCAL_EVENT_KEY1;
	}

	_keyTrigger = (keyEvent ^ _keyContinue) & keyEvent;
	_keyContinue = keyEvent;

	if(_keyTrigger != 0)
	{
		osSignalSet(_keyTid, _keyTrigger);
	}
}

#endif

/**
 * @brief 按键检测Task
 * @param args
 * TODO 修改为使用定时扫描的检测方式
 */
static void _KeyDetectTask(void const *args)
{
	osEvent event;

#if ! _KEY_DETECT_METHOD_INTERRUPT
	osTimerId timid;

	_keyTrigger = 0;
	_keyContinue = 0;

	osTimerDef(keyScan, _KeyScan);
	timid = osTimerCreate(osTimer(keyScan), osTimerPeriodic, NULL);
	osTimerStart(timid, 50);
#endif

	while(1)
	{
		event = osSignalWait(_LOCAL_EVENT_KEY0 | _LOCAL_EVENT_KEY1, osWaitForever);
		if(event.status != osEventSignal)
		{
			Util_NVIC_Cmd(EXTI0_IRQn, ENABLE);
			Util_NVIC_Cmd(EXTI1_IRQn, ENABLE);
			continue;
		}

		if(event.value.signals & _LOCAL_EVENT_KEY0)
		{
			osDelay(20); // 去抖动
			if(_KEY_PRESSED(_IO_KEY0))
			{
				DEBUG_MSG("K0\n");
				Audio_SetCalibrationOn();
			}

			Util_NVIC_Cmd(EXTI0_IRQn, ENABLE);
		}

		if(event.value.signals & _LOCAL_EVENT_KEY1)
		{
			osDelay(20); // 去抖动
			if(_KEY_PRESSED(_IO_KEY1))
			{
				DEBUG_MSG("K1\n");
			}

			Util_NVIC_Cmd(EXTI1_IRQn, ENABLE);
		}
	}
}

