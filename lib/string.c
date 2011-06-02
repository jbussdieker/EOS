////////////////////////////////////////////////////////////////////////////////
// $Id: string.c,v 1.5 2010/12/21 05:09:02 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <string.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void *memset(void *s, int c, size_t n)
{
	assert(s!=0);
	
	asm("cld\n" "rep\n" "stosb" : : "c" (n), "a" (c), "D" (s));
	return s;
}

void *memsetd(void *dest, unsigned long val, unsigned long count)
{
	assert(dest!=0);

	asm("cld\n" "rep\n" "stosl" : : "c" (count), "a" (val), "D" (dest));
	return dest;
}

void *memcpy(void *dest, const void *src, size_t count)
{
	assert(dest!=0); assert(src!=0);

	if ((count % 4) == 0)
		asm("cld\n" "rep\n" "movsl" : : "D" (dest), "S" (src), "c" (count/4));
	else if ((count % 2) == 0)
		asm("cld\n" "rep\n" "movsw" : : "D" (dest), "S" (src), "c" (count/2));
	else
		asm("cld\n" "rep\n" "movsb" : : "D" (dest), "S" (src), "c" (count));
	return dest;
}

int strcmp(const char *str1, const char *str2)
{
	assert(str1!=0); assert(str2!=0);
	
	unsigned long pos = 0;
	while ((str1[pos] == str2[pos]) &&	// Strings differ?
			 (str1[pos] != 0) && 					// String 1 end?
			 (str2[pos] != 0))						// String 2 end?
	pos++; // Move forward

	if (str1[pos] == str2[pos])
		return 0;
	else
		return str1[pos] - str2[pos];
}

int strncmp(const char *str1, const char *str2, size_t count)
{
	assert(str1!=0); assert(str2!=0);

	unsigned long pos = 0;
	while ((str1[pos] == str2[pos]) &&	// Strings differ?
			(str1[pos] != 0) &&					// String 1 end?
			(str2[pos] != 0) &&					// String 2 end?
			(pos < count))						// Count exceeded?
	pos++; // Move forward

	if (count == pos)
		return 0;
	else
		return str1[pos] - str2[pos];
}

size_t strnlen(const char *s, size_t maxlen)
{
	if (s==0)
		return 0;
		
	const char *es = s;
	while (*es && maxlen) {
		es++;
		maxlen--;
	}

	return (es - s);
}

size_t strlen(const char *str)
{
	assert(str!=0);

	int count = 0;
	while (str[count] != 0)
		count++;
	return count;
}

char *strcpy(char *dest, const char *src)
{
	assert(dest!=0); assert(src!=0);
	
	return memcpy(dest, src, strlen(src)+1);
}

char *strncpy(char *dest, const char *src, size_t maxlen)
{
	if (strlen(src) > maxlen)
		return memcpy(dest, src, maxlen);
	else
		return memcpy(dest, src, strlen(src)+1);
}

char *strchr(const char *str, int c)
{
	unsigned long pos = 0;
	
	while (str[pos] != c && str[pos] != 0)
		pos++;

	if (str[pos] == c)
		return (char *)&str[pos];
	else
		return 0;
}
