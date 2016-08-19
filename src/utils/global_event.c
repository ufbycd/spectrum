/*
 * global_event.c
 *
 *  Created on: 2016年8月18日
 *      Author: chenss
 */


#include "global_event.h"
#include "event_groups.h"

static EventGroupHandle_t _eventGroupHandel;

osStatus Util_GlobalEvent_Init(void)
{
	_eventGroupHandel = xEventGroupCreate();
	if(_eventGroupHandel != NULL)
	{
		return osOK;
	}
	else
	{
		return osErrorNoMemory;
	}
}

osStatus Util_GlobalEvent_Destory(void)
{
	vEventGroupDelete(_eventGroupHandel);
	return osOK;
}

osStatus Util_GlobalEvent_Set(uint32_t events)
{
	EventBits_t uxBits;
	BaseType_t xHigherPriorityTaskWoken, xResult;

	if(Util_IsInHandlerMode())
	{
		xHigherPriorityTaskWoken = pdFALSE;
		if(xEventGroupSetBitsFromISR(_eventGroupHandel, events, & xHigherPriorityTaskWoken) != pdPASS)
		{
			return osErrorOS;
		}

		portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
	else
	{
		uxBits = xEventGroupSetBits(_eventGroupHandel, (EventBits_t) events);
		return ((uxBits & events) == events) ? osOK : osErrorOS;
	}

	return osOK;
}

osEvent Util_GlobalEvent_Wait(int32_t events, uint32_t millisec)
{
	osEvent ret;
	TickType_t ticks;



	return ret;
}
