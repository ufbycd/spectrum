/************************************************************************
 *     COPYRIGHT NOTICE
 *     Copyright (c)  巴贝智慧科技有限公司	（版权声明）
 *     All rights reserved.
 *
 * @file	circular_buffer.h
 * @author	chenss
 * @date	2013年9月15日
 * @brief	
 *
 *（本文件实现的功能的详述）
 *
 *
************************************************************************/

#ifndef CIRCULAR_BUFFER_H_
#define CIRCULAR_BUFFER_H_

#include "main.h"


typedef uint8_t elem_t;

/* Circular buffer object */
typedef struct
{
	unsigned int size; /* maximum number of elements           */
	unsigned int start; /* index of oldest element              */
	unsigned int end; /* index at which to write new element  */
	elem_t *elems; /* vector of elements                   */
} cbuf_t;

void CBUF_Init(cbuf_t *cb, void *buf, unsigned int size);
int8_t CBUF_Write(cbuf_t *cb, void *e);
int8_t CBUF_Read(cbuf_t *cb, void *e);
void CBUF_Clear(cbuf_t *cb);


#endif /* CIRCULAR_BUFFER_H_ */
