////////////////////////////////////////////////////////////////////////////////
// $Id: stdio.c,v 1.8 2010/12/18 02:05:19 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <kernel/fs.h>

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
void **stdin;
extern int ttystartup;
void **stdout = (void *)&ttystartup;
void **stderr = (void *)&ttystartup;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
int getchar(void)
{
	unsigned char c;
	fs_read(*stdin, &c, 1);
	return c;	
}

int puts(const char *msg)
{
	int ret;
	ret = fs_write(*stdout, (void *)msg, strlen(msg));
	return ret;
}

int printf(const char *format, ...)
{
	char finalbuffer[128];

	void *args;
	va_start(args, format);
	vsprintf(finalbuffer, format, args);	
	va_end(args);

	return puts(finalbuffer);
}

int sprintf(char *buffer, const char *format, ...)
{
	void *args;
	int r;
	va_start(args, format);
	r = vsprintf(buffer, format, args);	
	va_end(args);

	return r;
}

int fprintf(FILE *stream, const char *format, ...)
{
	char finalbuffer[128];

	void *args;
	va_start(args, format);
	vsprintf(finalbuffer, format, args);	
	va_end(args);

	return fs_write(stream, finalbuffer, strlen(finalbuffer));
}

int fclose(FILE *stream)
{
	return fs_close(stream);
}

FILE *fopen(const char *filename, const char *mode)
{
	return fs_open(fsroot, filename);
}





static int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SMALL	32		/* Must be 32 == 0x20 */
#define SPECIAL	64		/* 0x */

#define __do_div(n, base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })

static char *number(char *str, long num, int base, int size, int precision,
		    int type)
{
	/* we are called with base 8, 10 or 16, only, thus don't need "G..."  */
	static const char digits[16] = "0123456789ABCDEF"; /* "GHIJKLMNOPQRSTUVWXYZ"; */

	char tmp[66];
	char c, sign, locase;
	int i;

	/* locase = 0 or 0x20. ORing digits or letters with 'locase'
	 * produces same digits or (maybe lowercased) letters */
	locase = (type & SMALL);
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return NULL;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else
		while (num != 0)
			tmp[i++] = (digits[__do_div(num, base)] | locase);
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
		while (size-- > 0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL) {
		if (base == 8)
			*str++ = '0';
		else if (base == 16) {
			*str++ = '0';
			*str++ = ('X' | locase);
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;
	while (i < precision--)
		*str++ = '0';
	while (i-- > 0)
		*str++ = tmp[i];
	while (size-- > 0)
		*str++ = ' ';
	return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int i, base;
	char *str;
	const char *s;

	int flags;		/* flags to number() */

	int field_width;	/* width of output field */
	int precision;		/* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;		/* 'h', 'l', or 'L' for integer fields */

	for (str = buf; *fmt; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
	      repeat:
		++fmt;		/* this also skips first '%' */
		switch (*fmt) {
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char)va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			continue;

		case 's':
			s = va_arg(args, char *);
			len = strnlen(s, precision);

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str,
				     (unsigned long)va_arg(args, void *), 16,
				     field_width, precision, flags);
			continue;

		case 'n':
			if (qualifier == 'l') {
				long *ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int *ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			*str++ = '%';
			continue;

			/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'x':
			flags |= SMALL;
		case 'X':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}
		if (qualifier == 'l')
			num = va_arg(args, unsigned long);
		else if (qualifier == 'h') {
			num = (unsigned short)va_arg(args, int);
			if (flags & SIGN)
				num = (short)num;
		} else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);
		str = number(str, num, base, field_width, precision, flags);
	}
	*str = '\0';
	return str - buf;
}




/*int vsprintf(char *buffer, const char *frmt, va_list args)
{
	assert(buffer!=0); assert(frmt!=0);

	const char *formatstring = frmt;
	char *outputbuffer = buffer;
	int width;
	int precision;
	char pad;
	int sharpmode;
	
	int always_show_sign;
	int left_align;

	while (*formatstring != 0)
	{
		width = 0;
		precision = 0;
		pad = ' ';
		
		// Flags
		always_show_sign = 0;
		left_align = 0;
		sharpmode = 0;
		
		switch (*formatstring)
		{
			case '%':
			{
				// %[parameter][flags][width][.precision][length]type
				
				
				
				// read flags (+,-, ,#)
				formatstring++;
				switch (*formatstring)
				{
					case '+':
						always_show_sign = 1;
						formatstring++;
						break;
					case '-':
						left_align = 1;
						formatstring++;
						break;
					case ' ':
						formatstring++;
						break;
					case '#':
						sharpmode = 1;
						formatstring++;
						break;
				}

				// read width (0|*|width.precision)
				if (*formatstring == '0')
					pad = '0';
				if (*formatstring == '*')
				{
					width = va_arg(args, int);
					formatstring++;
				}
				else
				{
					width = atoi(formatstring);					
					while (isdigit(*formatstring))
						formatstring++;
				}
				
				// Read precision
				if (*formatstring == '.')
				{
					formatstring++;
					precision = atoi(formatstring);					
					while (isdigit(*formatstring))
						formatstring++;
				}
				
				// Read modifiers (h, l, L)
				switch (*formatstring)
				{
					//Short Int
					case 'h':
						formatstring++;
						break;
					//Long Int
					case 'l':
						formatstring++;
						break;
					//Long Double
					case 'L':
						formatstring++;
						break;
				}
				
				// Read type (c,d,i,e,E,f,g,G,o,s,u,x,X,p,b,n)
				switch (*formatstring)
				{
					//Character
					case 'c':
					{
						*outputbuffer = va_arg(args, char);
						formatstring++;
						outputbuffer++;
						break;
					}
					//Signed Decimal Integer
					case 'd':
					case 'i':
					{
						char buffer[11]; // Max decimal 4 294 967 295
						char *ret;
						int i;
						i = va_arg(args, int);
						ret = itoa(i, buffer, 10);
						
						// Check if the padding flag is set
						if (width > 0 && strlen(ret) < width)
						{
							int fillsize = width - strlen(ret);
							ret -= width - strlen(ret);
							memset(ret, pad, fillsize);
						}
						
						strcpy(outputbuffer, ret);
						formatstring++;
						outputbuffer += strlen(ret);
						break;
					}
					//Scientific Notation
					case 'e':
					case 'E':
						formatstring++;
						break;
					//Decimal floating point
					case 'f':
						formatstring++;
						break;
					//Use shorter %e or %f
					case 'g':
					//Use shorter %E or %f
					case 'G':
						formatstring++;
						break;
					//Signed octal
					case 'o':
					{
						int i;
						char buffer[9]; // Max hex FFFF FFFF
						char *ret;
						i = va_arg(args, int);
						ret = itoa(i, buffer, 8);
						if (width > 0 && strlen(ret) < width)
						{
							int fillsize = width - strlen(ret);
							ret -= width - strlen(ret);
							memset(ret, pad, fillsize);
						}
						if (sharpmode == 1)
						{
							ret-=1;
							ret[0] = '0';
						}
						strcpy(outputbuffer, ret);
						formatstring++;
						outputbuffer += strlen(ret);
						break;
					}
					//String of characters
					case 's':
					{
						char *str = va_arg(args, char *);
						if (str != 0)
						{
							if (width > 0)
							{
								memcpy(outputbuffer, str, width);
								if (strlen(str) < width)
									memset(outputbuffer+strlen(str), pad, width - strlen(str));
								outputbuffer += width;
							}
							else
							{
								strcpy(outputbuffer, str);
								outputbuffer += strlen(str);
							}
						}
						formatstring++;
						break;
					}
					//Unsigned decimal integer
					case 'u':
					{
						char buffer[11]; // Max decimal 4 294 967 295
						char *ret;
						int i;
						i = va_arg(args, int);
						ret = itoa(i, buffer, 10);
						
						// Check if the padding flag is set
						if (width > 0 && strlen(ret) < width)
						{
							int fillsize = width - strlen(ret);
							ret -= width - strlen(ret);
							memset(ret, pad, fillsize);
						}
						
						strcpy(outputbuffer, ret);
						formatstring++;
						outputbuffer += strlen(ret);
						break;
					}
					//Unsigned hexadecimal integer
					case 'x':
					case 'X':
					{
						int i;
						char buffer[11]; // Max hex FFFF FFFF
						char *ret;
						i = va_arg(args, int);
						ret = itoa(i, buffer, 16);

						if (width > 0 && strlen(ret) < width)
						{
							int fillsize = width - strlen(ret);
							ret -= width - strlen(ret);
							memset(ret, pad, fillsize);
						}
						if (sharpmode == 1)
						{
							ret-=2;
							ret[0] = '0';
							ret[1] = *formatstring;
						}

						strcpy(outputbuffer, ret);
						formatstring++;
						outputbuffer += strlen(ret);
						break;
					}
					//Address pointed by the argument
					case 'p':
					{
						int i;
						char buffer[9]; // Max hex FFFF FFFF
						char *ret;
						i = va_arg(args, int);
						ret = itoa(i, buffer, 16);
						strcpy(outputbuffer, ret);
						outputbuffer += strlen(ret);
						formatstring++;
						break;
					}
					case 'b':
					{
						formatstring++;
						int i = va_arg(args, int);
						char buffer[33]; // Max binary 11111111 11111111 11111111 11111111
						char *ret;
						ret = itoa(i, buffer, 2);

						if (width > 0 && strlen(ret) < width)
						{
							int fillsize = width - strlen(ret);
							ret -= width - strlen(ret);
							memset(ret, pad, fillsize);
						}

						strcpy(outputbuffer, ret);
						outputbuffer += strlen(ret);
						break;
					}
					//Nothing printed. The argument must be a pointer to integer where the number of characters written so far will be stored.
					case 'n':
						//printf("nothing printed. The argument must be a pointer to integer where the number of characters written so far will be stored.*\n");
						formatstring++;
						break;
				}
				break;
			}
			default:
			{
				*outputbuffer = *formatstring;
				formatstring++;
				outputbuffer++;
			}
		}
	}

	*outputbuffer = '\0';

	return strlen(buffer);
}*/
