////////////////////////////////////////////////////////////////////////////////
// $Id: stdlib.c,v 1.2 2010/12/17 23:54:49 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <assert.h>

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
int atoi(char *buffer)
{
	assert(buffer!=0);
	
	int i = 0;
	int ret = 0;
	int base = 10; // Hex or Decimal?
	
	// Check for hex formating otherwise default to decimal
	if (buffer[0] == '0' && buffer[1] == 'x')
	{
		base = 16;
		i = 2;
	}
	
	while (buffer[i] != 0)
	{
		if (buffer[i] >= '0' && buffer[i] <= '9')
			ret = ret * base + buffer[i] - '0';
		else if (buffer[i] >= 'a' && buffer[i] <= 'f' && base == 16)
			ret = ret * base + buffer[i] - 'a' + 10;
		else if (buffer[i] >= 'A' && buffer[i] <= 'F' && base == 16)
			ret = ret * base + buffer[i] - 'A' + 10;
		else
			break;
		i++;
	}
	return ret;
}

char *itoa(int num, char *buffer, int radix)
{
	assert(buffer!=0);

	char *back = 0;
	unsigned long val = num;
	switch (radix)
	{
		case 2:
		{
			back = buffer + 32;
			*back = 0;
			do {
					back--;
				*back = (val & 0x1) + '0';
				val = val >> 1;
			} while (val);
			break;
		}
		case 8:
		{
			if (num<0)
				val = -num;
				
			back = buffer + 10;
			*back = 0;
			do {
				*--back = val % 8 + '0';
				val /= 8;
			} while (val);
			if (num<0)
				*--back = '-';
			break;
		}
		case 10:
		{
			if (num<0)
				val = -num;
				
			back = buffer + 10;
			*back = 0;
			do {
				*--back = val % 10 + '0';
				val /= 10;
			} while (val);
			if (num<0)
				*--back = '-';
			break;
		}
		case 16:
		{
			back = buffer + 8;
			*back = 0;
			do {
				back--;
				if ((val & 0xF) > 0x9)
					*back = (val & 0xF) + 'A' - 10;
				else
				*back = (val & 0xF) + '0';
			val = val >> 4;
			} while (val);
			break;
		}
		default:
			break;
	}
	return back;
}
