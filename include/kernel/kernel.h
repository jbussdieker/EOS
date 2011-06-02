////////////////////////////////////////////////////////////////////////////////
// $Id: kernel.h,v 1.13 2010/12/28 02:23:08 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////
#ifndef _KERNEL_H
#define _KERNEL_H

typedef unsigned long uint32;
typedef unsigned long dword;

typedef unsigned short uint16;
typedef unsigned short word;

typedef unsigned char uint8;
typedef unsigned char byte;

typedef long int32;
typedef long sdword;

typedef short int16;
typedef short sword;

typedef char sbyte;
typedef char int8;

typedef struct kernel_t
{
	int argc;
	char *args;
	
	void *start;
	void *end;
	volatile unsigned long ticks;
	unsigned long memory;
	char cpuname[64];
	char buildstring[128];

	volatile int gui;
	int loglevel;
	int headless;
} kernel_t;

extern kernel_t kinfo;

void kernel_init();

#define kprintf(level, format, va...) \
	if (level <= kinfo.loglevel) \
	{ \
		printf("%*s(%d) ", level, " ", level); \
		printf(format, va); \
	}
//#define kprintf(level, format, va...) 0

#endif
