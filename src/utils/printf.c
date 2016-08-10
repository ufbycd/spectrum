/*
 Copyright 2001, 2002 Georges Menie (www.menie.org)
 stdarg version contributed by Christian Ettinger

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 putchar is the only external dependency for this file,
 if you have a working putchar, leave it commented out.
 If not, uncomment the define below and
 replace outbyte(c) by your own function call.

 #define putchar(c) outbyte(c)
 */

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include "serial.h"


#define SUPPOR_LONG_LONG          0
#define PRINT_FLOAT_AGINST_DOUBLE 0
#if ! PRINT_FLOAT_AGINST_DOUBLE
#	undef SUPPOR_LONG_LONG
#	define SUPPOR_LONG_LONG 1
#endif

struct _buf
{
	int 	isSizeLimited;
	size_t 	size;
	char 	*data;
};

static int printchar(struct _buf *sbuf, int c)
{
	if(sbuf)
	{
		if(sbuf->isSizeLimited)
		{
			if(sbuf->size < 1)
				return -1;
			else if(sbuf->size == 1)
			{
				*(sbuf->data) = '\0';
				sbuf->size = 0;
				return 1;
			}
		}

		*(sbuf->data) = (char)c;
		sbuf->data += 1;
		sbuf->size -= 1;
	}
	else
		Serial_PutChar(c);

	return 1;
}

#define PAD_RIGHT 1
#define PAD_ZERO 2

static int prints(struct _buf *sbuf, const char *string, int width, int pad)
{
	register int pc = 0, padchar = ' ';

	if(width > 0)
	{
		register int len = 0;
		register const char *ptr;
		for(ptr = string; *ptr; ++ptr)
			++len;
		if(len >= width)
			width = 0;
		else
			width -= len;
		if(pad & PAD_ZERO)
			padchar = '0';
	}
	if(!(pad & PAD_RIGHT))
	{
		for(; width > 0; --width)
		{
			printchar(sbuf, padchar);
			++pc;
		}
	}
	for(; *string; ++string)
	{
		printchar(sbuf, *string);
		++pc;
	}
	for(; width > 0; --width)
	{
		printchar(sbuf, padchar);
		++pc;
	}

	return pc;
}

/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 24

static int printInt(struct _buf *sbuf, int value, int ary, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *str;
	register int t, neg = 0, pc = 0;
	register unsigned int u = value;

	if(value == 0)
	{
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(sbuf, print_buf, width, pad);
	}

	if(sg && ary == 10 && value < 0)
	{
		neg = 1;
		u = -value;
	}

	str = print_buf + PRINT_BUF_LEN - 1;
	*str = '\0';

	while(u)
	{
		t = u % ary;
		if(t >= 10)
			t += letbase - '0' - 10;
		*--str = t + '0';
		u /= ary;
	}

	if(neg)
	{
		if(width && (pad & PAD_ZERO))
		{
			printchar(sbuf, '-');
			++pc;
			--width;
		}
		else
		{
			*--str = '-';
		}
	}

	return pc + prints(sbuf, str, width, pad);
}

#if SUPPOR_LONG_LONG
static int printLongLongInt(struct _buf *sbuf, long long int value, int ary, int sg, int width, int pad, int letbase)
{
	char print_buf[PRINT_BUF_LEN];
	register char *str;
	register int t, neg = 0, pc = 0;
	register unsigned long long int u = value;

	if(value == 0)
	{
		print_buf[0] = '0';
		print_buf[1] = '\0';
		return prints(sbuf, print_buf, width, pad);
	}

	if(sg && ary == 10 && value < 0)
	{
		neg = 1;
		u = -value;
	}

	str = print_buf + PRINT_BUF_LEN - 1;
	*str = '\0';

	while(u)
	{
		t = u % ary;
		if(t >= 10)
			t += letbase - '0' - 10;
		*--str = t + '0';
		u /= ary;
	}

	if(neg)
	{
		if(width && (pad & PAD_ZERO))
		{
			printchar(sbuf, '-');
			++pc;
			--width;
		}
		else
		{
			*--str = '-';
		}
	}

	return pc + prints(sbuf, str, width, pad);
}
#else
#	define printLongLongInt(sbuf, value, ary, sg, width, pad, letbase) 0
#endif

#if ! PRINT_FLOAT_AGINST_DOUBLE
static int printDouble(struct _buf *sbuf, double value, int ary, int width, int precision, int pad, int letbase)
{
	int num;
	long long int llvalue, lli;
	unsigned long long ull;
	int i;

	num = 0;

	llvalue = (long long int) value;
	num += printLongLongInt(sbuf, llvalue, ary, 1, 0, PAD_RIGHT, letbase);

	printchar(sbuf, '.');
	num += 1;

	lli = 1;
	for(i = 0; i < precision; i++)
	{
		lli *= 10;
	}

	value -= llvalue;
	llvalue = (long long int) (value * lli);
	ull = (llvalue > 0) ? llvalue : - llvalue;
	num += printLongLongInt(sbuf, ull, ary, 1, 0, PAD_RIGHT, letbase);

	return num;
}
#else
static int printFloat(struct _buf *sbuf, float value, int ary, int width, int precision, int pad, int letbase)
{
	int num;
	int32_t llvalue, lli;
	uint32_t uli;
	int i;

	num = 0;

	llvalue = (int32_t) value;
	num += printInt(sbuf, llvalue, ary, 1, 0, PAD_RIGHT, letbase);

	printchar(sbuf, '.');
	num += 1;

	lli = 1;
	for(i = 0; i < precision; i++)
	{
		lli *= 10;
	}

	value -= llvalue;
	llvalue = (int32_t) (value * lli);
	uli = (llvalue > 0) ? llvalue : - llvalue;
	num += printInt(sbuf, uli, ary, 1, 0, PAD_RIGHT, letbase);

	return num;
}
#endif

/**
 * @brief A format specifier follows this prototype:
 * %[flags][width][.precision][length]specifier
 *
 * @param out
 * @param format
 * @param args
 * @return
 */
static int print(struct _buf *sbuf, const char *format, va_list args)
{
	register int width, precision, pad;
	register int num = 0;
	char scr[2];
	char c, *pchar;

	enum _length
	{
		_normal = 0,
		_short,
		_short_short,
		_long,
		_long_long
	} length;


	for(; *format != 0; ++format)
	{

		if(*format != '%')
		{
			printchar(sbuf, *format);
			++num;
		}
		else if(format[1] == '%')
		{
			printchar(sbuf, '%');
			++format;
			++num;
		}
		else
		{
			++format;
			width = 0;
			precision = 0;
			length = _normal;
			pad = 0;

			c = *format;
			if(c == '\0')
				break;

			/* parse flags */
			switch(c)
			{
			case '-':
				pad = PAD_RIGHT;
				++format;
				break;
			case '+':
			case '#':
				++format;
				break;
			case '0':
				pad |= PAD_ZERO;
				++format;
				break;
			}

			/* Parse width */
			while((*format >= '0') && (*format <= '9'))
			{
				width *= 10;
				width += *format - '0';
				++format;
			}

			/* Parse precision */
			if(*format == '.')
			{
				++format;
				while((*format >= '0') && (*format <= '9'))
				{
					precision *= 10;
					precision += *format - '0';
					++format;
				}
			}
			else
			{
				precision = 6;
			}

			/* parse length */
			switch(*format)
			{
			case 'l':
				if(format[1] == 'l')
				{
					length = _long_long;
					++format;
				}
				else
					length = _long;

				++format;
				break;

			case 'h':
				if(format[1] == 'h')
					++format;
				/* no break */
			case 'L':
			case 'j':
			case 'z':
			case 't':
				++format;
				break;
			}

			int letbase;
			if(*format >= 'a')
				letbase = 'a';
			else
				letbase = 'A';

			/* parse specifier */
			switch(*format)
			{
			case 's':
				pchar = (char *) va_arg( args, int );
				num += prints(sbuf, pchar ? pchar : "(null)", width, pad);
				break;

			case 'd':
				if(length == _long_long)
					num += printLongLongInt(sbuf, va_arg( args, long long int ), 10, 1, width, pad, letbase);
				else if(length == _long)
					num += printInt(sbuf, va_arg( args, long int ), 10, 1, width, pad, letbase);
				else
					num += printInt(sbuf, va_arg( args, int ), 10, 1, width, pad, letbase);
				break;

			case 'x':
			case 'X':
				if(length == _long_long)
					num += printLongLongInt(sbuf, va_arg( args, unsigned long long int ), 16, 0, width, pad, letbase);
				else if(length == _long)
					num += printInt(sbuf, va_arg( args, unsigned long int ), 16, 0, width, pad, letbase);
				else
					num += printInt(sbuf, va_arg( args, unsigned int ), 16, 0, width, pad, letbase);
				break;

			case 'u':
				if(length == _long_long)
					num += printLongLongInt(sbuf, va_arg( args, unsigned long long int ), 10, 0, width, pad, letbase);
				else if(length == _long)
					num += printInt(sbuf, va_arg( args, unsigned long int ), 10, 0, width, pad, letbase);
				else
					num += printInt(sbuf, va_arg( args, unsigned int ), 10, 0, width, pad, letbase);
				break;

			case 'o':
				if(length == _long_long)
					num += printLongLongInt(sbuf, va_arg( args, unsigned long long int ), 8, 0, width, pad, letbase);
				else if(length == _long)
					num += printInt(sbuf, va_arg( args, unsigned long int ), 8, 0, width, pad, letbase);
				else
					num += printInt(sbuf, va_arg( args, unsigned int ), 8, 0, width, pad, letbase);
				break;

			case 'c':
				/** @note char are converted to int then pushed on the stack */
				scr[0] = (char) va_arg( args, int );
				scr[1] = '\0';
				num += prints(sbuf, scr, width, pad);
				break;

			case 'f':
			case 'F':
			case 'e':
			case 'E':
			case 'g':
			case 'G':
#if PRINT_FLOAT_AGINST_DOUBLE
				num += printFloat(sbuf, va_arg(args, float), 10, width, precision, pad, letbase);
#else
				num += printDouble(sbuf, va_arg(args, double), 10, width, precision, pad, letbase);
#endif
				break;
			}
		}
	}

	if(sbuf)
		printchar(sbuf, '\0');

	va_end(args);

	return num;
}

int printf(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	return print(NULL, format, args);
}

int vprintf(const char *format, va_list args)
{
	return print(NULL, format, args);
}

int sprintf(char *out, const char *format, ...)
{
	va_list args;
	struct _buf mybuf = {0, -1, out};

	va_start(args, format);
	return print(&mybuf, format, args);
}

int vsprintf(char *out, const char *format, va_list args)
{
	struct _buf mybuf = {0, -1, out};

	return print(&mybuf, format, args);
}

int snprintf(char *out, unsigned int size, const char *format, ...)
{
	va_list args;
	struct _buf mybuf = {1, size, out};

	va_start(args, format);
	return print(&mybuf, format, args);
}

int puts(const char *s)
{
	int i;
	char c;

	c = *s++;
	for(i = 0; c != '\0'; i++)
	{
		Serial_PutChar(c);
		c = *s++;
	}

	Serial_PutChar('\n');

	return i + 1;
}

#ifdef TEST_PRINTF
int main(void)
{
	char *ptr = "Hello world!";
	char *np = 0;
	int i = 5;
	unsigned int bs = sizeof(int)*8;
	int mi;
	char buf[80];

	mi = (1 << (bs-1)) + 1;
	printf("%s\n", ptr);
	printf("printf test\n");
	printf("%s is null pointer\n", np);
	printf("%d = 5\n", i);
	printf("%d = - max int\n", mi);
	printf("char %c = 'a'\n", 'a');
	printf("hex %x = ff\n", 0xff);
	printf("hex %02x = 00\n", 0);
	printf("signed %d = unsigned %u = hex %x\n", -3, -3, -3);
	printf("%d %s(s)%", 0, "message");
	printf("\n");
	printf("%d %s(s) with %%\n", 0, "message");
	sprintf(buf, "justif: \"%-10s\"\n", "left"); printf("%s", buf);
	sprintf(buf, "justif: \"%10s\"\n", "right"); printf("%s", buf);
	sprintf(buf, " 3: %04d zero padded\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %-4d left justif.\n", 3); printf("%s", buf);
	sprintf(buf, " 3: %4d right justif.\n", 3); printf("%s", buf);
	sprintf(buf, "-3: %04d zero padded\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %-4d left justif.\n", -3); printf("%s", buf);
	sprintf(buf, "-3: %4d right justif.\n", -3); printf("%s", buf);

	return 0;
}

/*
 * if you compile this file with
 *   gcc -Wall $(YOUR_C_OPTIONS) -DTEST_PRINTF -c printf.c
 * you will get a normal warning:
 *   printf.c:214: warning: spurious trailing `%' in format
 * this line is testing an invalid % at the end of the format string.
 *
 * this should display (on 32bit int machine) :
 *
 * Hello world!
 * printf test
 * (null) is null pointer
 * 5 = 5
 * -2147483647 = - max int
 * char a = 'a'
 * hex ff = ff
 * hex 00 = 00
 * signed -3 = unsigned 4294967293 = hex fffffffd
 * 0 message(s)
 * 0 message(s) with %
 * justif: "left      "
 * justif: "     right"
 *  3: 0003 zero padded
 *  3: 3    left justif.
 *  3:    3 right justif.
 * -3: -003 zero padded
 * -3: -3   left justif.
 * -3:   -3 right justif.
 */

#endif

