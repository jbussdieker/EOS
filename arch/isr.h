////////////////////////////////////////////////////////////////////////////////
// $Id: isr.h,v 1.1 2010/12/18 00:58:26 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////
#ifndef _ISR_H
#define _ISR_H

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
typedef struct context_t
{
	unsigned long gs;
	unsigned long fs;
	unsigned long es;
	unsigned long ds;

	unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;

	unsigned long int_no;
	unsigned long err_code;
	
	void *eip;
	unsigned long cs;
	unsigned long eflags;
	// Only valid on RING0->RING3
	void *uesp;
	unsigned long uss;
} context_t;

#endif