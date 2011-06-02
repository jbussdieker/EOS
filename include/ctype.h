////////////////////////////////////////////////////////////////////////////////
// $Id: ctype.h,v 1.1 2010/12/12 23:57:31 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////
#ifndef _CTYPE_H
#define _CTYPE_H

/*
isalnum		test for alphanumeric character
isalpha 	test for alphabetic character
isblank 	test for blank character (new in C99)
iscntrl 	test for control character
isdigit 	test for digit. Not locale-specific.
isgraph 	test for graphic character, excluding the space character.
islower 	test for lowercase character
isprint 	test for printable character, including the space character.
ispunct 	test for punctuation character
isspace 	test for any whitespace character
isupper 	test for uppercase character
isxdigit 	test for hexadecimal digit. Not locale-specific.

Character conversion 	in the form int tofunc(int);
Return the converted character unless it is not alphabetic.

tolower 	convert character to lowercase
toupper 	convert character to uppercase
*/

int   isalnum(int);
int   isalpha(int);
int   isblank(int);
int   iscntrl(int);
int   isdigit(int);
int   isgraph(int);
int   islower(int);
int   isprint(int);
int   ispunct(int);
int   isspace(int);
int   isupper(int);
int   isxdigit(int);
int   tolower(int);
int   toupper(int);

#endif