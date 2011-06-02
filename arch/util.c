////////////////////////////////////////////////////////////////////////////////
// $Id: util.c,v 1.1 2010/12/18 01:18:00 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void outportb(unsigned short port, unsigned char data)
{
	__asm__ __volatile__ ("outb %1, %0" : : "d" (port), "a" (data));
}

void outportw(unsigned short port, unsigned short data) 
{ 
	__asm__ __volatile__ ("outw %1, %0" : : "d" (port), "a" (data)); 
}

void outport(unsigned short port, unsigned long data) 
{ 
	__asm__ __volatile__ ("outl %1, %0" : : "d" (port), "a" (data)); 
}

unsigned char inportb(unsigned short port)
{
	unsigned char rv;
	__asm__ __volatile__ ("inb %1, %0" : "=a" (rv) : "dN" (port));
	return rv;
}

unsigned short inportw (unsigned short port)
{
	unsigned short rv;
	__asm__ __volatile__ ("inw %1, %0" : "=a" (rv) : "dN" (port));
	return rv;
}

unsigned long inport(unsigned short port)
{
	unsigned long rv;
	__asm__ __volatile__ ("inl %1, %0" : "=a" (rv) : "dN" (port));
	return rv;
}

void sti()
{
	__asm__ __volatile__ ("sti");
}

void cli()
{
	__asm__ __volatile__ ("cli");
}
