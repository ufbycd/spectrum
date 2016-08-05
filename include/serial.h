/*
 * serial.h
 *
 *  Created on: 2016年8月5日
 *      Author: chenss
 */

#ifndef SERIAL_H_
#define SERIAL_H_

#include "main.h"

void Serial_Init(void);
char Serial_GetChar(void);
void Serial_PutChar(char c);


#endif /* SERIAL_H_ */
