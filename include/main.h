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
//#include "printf.h"
#include "FreeRTOS.h"

#define FORCE_INLINE __inline__ __attribute__((always_inline))

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

#endif /* MAIN_H_ */
