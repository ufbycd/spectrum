/*
 * resource.c
 *
 *  Created on: 2016年8月8日
 *      Author: chenss
 */

#include "utils.h"
#include <stdlib.h>

#define FLAGS_CREATE_BY_ATTACH 0x1

struct Util_Resource
{
	unsigned int flags;
	osSemaphoreId semhr;
	void *buf;
	size_t size;
};

Util_ResourceHandle_t Util_ResourceCreate(size_t dataBufSize)
{
	Util_ResourceHandle_t resource;

	resource = malloc(sizeof(struct Util_Resource));
	configASSERT(resource);
	resource->flags = 0;

	osSemaphoreDef(semhr);
	resource->semhr = osSemaphoreCreate(osSemaphore(semhr), 1);
	configASSERT(resource->semhr);

	resource->buf = malloc(dataBufSize);
	configASSERT(resource->buf);

	resource->size = dataBufSize;

	return resource;
}

Util_ResourceHandle_t Util_ResourceAttach(void *dataBuf, size_t dataBufSize)
{
	Util_ResourceHandle_t resource;

	resource = malloc(sizeof(struct Util_Resource));
	configASSERT(resource);
	resource->flags = FLAGS_CREATE_BY_ATTACH;

	osSemaphoreDef(semhr);
	resource->semhr = osSemaphoreCreate(osSemaphore(semhr), 1);
	configASSERT(resource->semhr);

	resource->size = dataBufSize;

	return resource;
}

osStatus Util_ResourceDelete(Util_ResourceHandle_t resource)
{
	osStatus stat;

	if(! (resource->flags & FLAGS_CREATE_BY_ATTACH))
	{
		free(resource->buf);
	}

	stat = osSemaphoreDelete(resource->semhr);
	free(resource);

	return stat;
}

void * Util_ResourceGet(Util_ResourceHandle_t resource, uint32_t timeOutMs)
{
	osStatus stat;

	stat = osSemaphoreWait(resource->semhr, timeOutMs);
	if(stat != osOK)
	{
		configASSERT(false);
		Util_ResourceRelease(resource);
		return NULL;
	}

	return resource->buf;
}

osStatus Util_ResourceRelease(Util_ResourceHandle_t resource)
{
	osStatus stat;

	stat = osSemaphoreRelease(resource->semhr);
	configASSERT(stat == osOK);

	return stat;
}

size_t Util_ResourceSize(Util_ResourceHandle_t resource)
{
	return resource->size;
}
