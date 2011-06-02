////////////////////////////////////////////////////////////////////////////////
// $Id: pit.c,v 1.1 2010/12/23 09:22:01 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/arch.h>

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void pit_init(int hz)
{
	// Set the pit frequency
	int divisor = 1193180 / hz;
	outportb(0x43, 0x36);
	outportb(0x40, divisor & 0xFF);
	outportb(0x40, divisor >> 8);
}
