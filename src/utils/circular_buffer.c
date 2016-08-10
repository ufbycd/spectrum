/************************************************************************
 *     COPYRIGHT NOTICE
 *     Copyright (c)  巴贝智慧科技有限公司	（版权声明）
 *     All rights reserved.
 *
 * @file	circular_buffer.c
 * @author	chenss
 * @date	2013年9月15日
 * @brief	环形队列
 *
 *（本文件实现的功能的详述）
 *
 *
 ************************************************************************/
#include "circular_buffer.h"

/** 初始化环形队列
 *
 * @param cb 环形队列指针
 * @param buf 队列内容缓存
 * @param size 队内内容大小
 * @note  队内内容大小 必须为2的n次方
 */
void CBUF_Init(cbuf_t *cb, void *buf, unsigned int size)
{
	cb->size = size;
	cb->start = 0;
	cb->end = 0;
	cb->elems  = buf;
}

#if 1
#   define CBUF_IsFull(cb) (cb->end == (cb->start ^ cb->size))
#   define CBUF_IsEmpty(cb) (cb->end == cb->start)
#   define CBUF_Incr(cb, p) ((p + 1) & (2 * cb->size - 1))
#else

int8_t CBUF_IsFull(cbuf_t *cb)
{
    /* This inverts the most significant bit of start before comparison */
    return cb->end == (cb->start ^ cb->size);
}

int8_t CBUF_IsEmpty(cbuf_t *cb)
{
    return cb->end == cb->start;
}

int8_t CBUF_Incr(cbuf_t *cb, int8_t p)
{
    /* start and end pointers incrementation is done modulo 2*size */
    return (p + 1) & (2 * cb->size - 1);
}

void CBUF_Print(cbuf_t *cb)
{
    printf("size=0x%x, start=%d, end=%d\n", cb->size, cb->start, cb->end);
}
#endif

int8_t CBUF_Write(cbuf_t *cb, void *e)
{
	int8_t rel = 0;
	elem_t *elem = e;

	cb->elems[cb->end & (cb->size - 1)] = *elem;

	if(CBUF_IsFull(cb))
	{
		return -1;
	}

	cb->end = CBUF_Incr(cb, cb->end);

	return rel;
}

int8_t CBUF_Read(cbuf_t *cb, void *e)
{
	elem_t *elem = e;

	if(CBUF_IsEmpty(cb))
	{
		return -1;
	}
	else
	{
		*elem = cb->elems[cb->start & (cb->size - 1)];
		cb->start = CBUF_Incr(cb, cb->start);

		return 0;
	}
}

void CBUF_Clear(cbuf_t *cb)
{
    cb->end = 0;
    cb->start = 0;
}
