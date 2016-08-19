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


