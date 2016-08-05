/*
 * utils.h
 *
 *  Created on: 2016年8月5日
 *      Author: chenss
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "main.h"

#define UTILS_MS_TICKS(ms) ((TickType_t)(ms) / portTICK_RATE_MS)

void Utils_DelayMs(uint32_t ms);


#endif /* UTILS_H_ */
