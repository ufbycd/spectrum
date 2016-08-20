/*
 * LedMatrix.h
 *
 *  Created on: 2016年8月15日
 *      Author: chenss
 */

#ifndef LEDMATRIX_H_
#define LEDMATRIX_H_


#include "main.h"


void LedMatrix_Init(void);
void LedMatrix_SetBrightness(float level);

osThreadId LedMatrix_GetDisplayThreadId(void);

#endif /* LEDMATRIX_H_ */
