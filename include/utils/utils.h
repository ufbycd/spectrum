/*
 * utils.h
 *
 *  Created on: 2016年8月5日
 *      Author: chenss
 */

#ifndef UTILS_H_
#define UTILS_H_

#include "main.h"

typedef struct Util_Resource * Util_ResourceHandle_t;

Util_ResourceHandle_t Util_ResourceCreate(size_t dataBufSize);
Util_ResourceHandle_t Util_ResourceAttach(void *dataBuf, size_t dataBufSize);
osStatus Util_ResourceDelete(Util_ResourceHandle_t resource);

void * Util_ResourceGet(Util_ResourceHandle_t resource, uint32_t timeOutMs);
osStatus Util_ResourceRelease(Util_ResourceHandle_t resource);
size_t Util_ResourceSize(Util_ResourceHandle_t resource);

int Util_IsInHandlerMode (void);

#endif /* UTILS_H_ */
