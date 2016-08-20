/*
 * util.c
 *
 *  Created on: 2016年8月18日
 *      Author: chenss
 */

#include "utils.h"

/* Determine whether we are in thread mode or handler mode. */
int Util_IsInHandlerMode (void)
{
  return __get_IPSR() != 0;
}


void Util_NVIC_Cmd(uint8_t NVIC_IRQChannel, FunctionalState NVIC_IRQChannelCmd)
{
	if (NVIC_IRQChannelCmd != DISABLE)
	{
		/* Enable the Selected IRQ Channels --------------------------------------*/
		NVIC->ISER[NVIC_IRQChannel >> 0x05] = (uint32_t) 0x01
				<< (NVIC_IRQChannel & (uint8_t) 0x1F);
	}
	else
	{
		/* Disable the Selected IRQ Channels -------------------------------------*/
		NVIC->ICER[NVIC_IRQChannel >> 0x05] = (uint32_t) 0x01
				<< (NVIC_IRQChannel & (uint8_t) 0x1F);
	}
}

