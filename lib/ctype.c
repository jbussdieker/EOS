////////////////////////////////////////////////////////////////////////////////
// $Id: ctype.c,v 1.3 2010/12/28 06:11:58 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <ctype.h>

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
int isalnum(int c)
{
	return (isalpha(c) || isdigit(c));
}

int isalpha(int c)
{
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))
		return 1;
	else
		return 0;
}

int isblank(int c)
{
	return 0;
}

int iscntrl(int c)
{
	if ((c >= 0x00 && c <= 0x1F) || c == 0x7F)
		return 1;
	else
		return 0;
}

int isdigit(int c)
{
	if (c >= '0' && c <= '9')
		return 1;
	else
		return 0;
}

int isgraph(int c)
{
	if (c > 0x20 && c < 0x7F)
		return 1;
	else
		return 0;
}

int islower(int c)
{
	if (c >= 'a' && c <= 'z')
		return 1;
	else
		return 0;
}

int isprint(int c)
{
	return (isgraph(c) || c == ' ');
}

int ispunct(int c)
{
	if (isprint(c) && (isalnum(c) == 0))
		return 1;
	else
		return 0;
}

int isspace(int c)
{
	if (c == ' ' ||c == '\t' ||  c == '\v' || c == '\f' || c == '\r' || c == '\n')
		return 1;
	else
		return 0;
}

int isupper(int c)
{
	if (c >= 'A' && c <= 'Z')
		return 1;
	else
		return 0;
}

int isxdigit(int c)
{
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
		return 1;
	else
		return 0;
}

int tolower(int c)
{
	return 0;
}

int toupper(int c)
{
	return 0;
}
