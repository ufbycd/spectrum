/*
 * main.h
 *
 *  Created on: 2013-5-28
 *      Author: chenss
 */

#ifndef MAIN_H_
#define MAIN_H_

#ifdef DEBUG
#   include <stdio.h>
#endif

#include <stdbool.h>

//#define NDEBUG
#include <assert.h>

#include "stm32f10x.h"
#include "cmsis_os.h"
#include "utils.h"

#define FORCE_INLINE __inline__ __attribute__((always_inline))

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#define BIT_SET(reg, bit) (reg) |= (bit)
#define BIT_CLR(reg, bit) (reg) &= ~ (bit)
#define BITS_SET(reg, mask, bits) do{reg = (reg & (~(mask))) | (bits);}while(0);


#define ENABLE_COLOR 1

#define BLACK 		"\x1b[30m"
#define RED 		"\x1b[31m"
#define GREEN 		"\x1b[32m"
#define YELLOW 		"\x1b[33m"
#define BLUE 		"\x1b[34m"
#define MAGENTA 	"\x1b[35m"
#define CYAN 		"\x1b[36m"
#define WHILE		"\x1b[37m"
#define COLOR_END	"\x1b[0m"

#if ENABLE_COLOR
#	define COLOR_TXT(txt, color) color txt "\x1b[0m"
#else
#	define COLOR_TXT(txt, color) txt
#endif

int printSafe(const char *fmt, ...);

#ifdef DEBUG
#   define DEBUG_MSG(fmt, args...) printf(fmt, ##args)
#   define MDEBUG(fmt, args...) printf(fmt, ##args)
#   define MDEBUG_COLOR(color, fmt, args...) printf(COLOR_TXT(fmt, color), ##args)
#   define ON_DEBUG(f)  f
#else
#   define DEBUG_MSG(fmt, args...)
#   define MDEBUG(fmt, args...)
#   define MDEBUG_COLOR(color, fmt, args...)
#   define ON_DEBUG(f)
#endif



#define EVENT_SAMPLE_FINISH 0x1
#define EVENT_FRAME_BEGIN   (0x1 << 1)
#define EVENT_FFT_IN_FILL   (0x1 << 2)
#define EVENT_SPECTRUM_FILL (0x1 << 3)


#endif /* MAIN_H_ */
