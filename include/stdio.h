////////////////////////////////////////////////////////////////////////////////
// $Id: stdio.h,v 1.4 2010/12/17 01:16:44 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////
#ifndef _STDIO_H
#define _STDIO_H

#include <stdarg.h>

#define FILENAME_MAX 128

typedef void FILE;

extern void **stdin;
extern void **stdout;
extern void **stderr;

int puts(const char *str);
int printf(const char *, ...);
int fprintf(FILE *, const char *, ...);
int getchar(void);
int sprintf(char *, const char *, ...);
int fclose(FILE *);
FILE *fopen(const char *, const char *);
int vsprintf(char *buf, const char *fmt, va_list args);

#endif
