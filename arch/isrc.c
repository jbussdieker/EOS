////////////////////////////////////////////////////////////////////////////////
// $Id: isrc.c,v 1.4 2010/12/23 09:22:01 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <kernel/arch.h>
#include <kernel/syscall.h>
#include <kernel/elf.h>
#include <kernel/tasks.h>
#include <assert.h>
#include <stdio.h>
#include "isr.h"
#include "fault.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
static void *irq_handlers[16] =
{
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void arch_register_irq(int int_no, void *handler)
{
	assert(int_no < 16);
	
	// Set an IRQ handler callback function pointer
	//printf("i386_hook_int(%d, %08x)\n", int_no, handler);
	irq_handlers[int_no] = handler;
}

void debug(context_t *r)
{
	printf("%08x %s\n", r->eip, elf_get_symbol(r->eip));
}

void irq(context_t *r)
{
	int irq = r->int_no - 32;
	if (irq == 0)
	{
		task_scheduler();
		kinfo.ticks++;
	}
	else
	{
		// Call the IRQ handler
		void (*func)(context_t *r);
		func = irq_handlers[irq];
		if (func != 0)
			func(r);
	}
	
	// Acknowledge hardware interrupts
	pic_ack(irq);
}

void isr(context_t *r)
{
	// Debug
	if (r->int_no == 1)
		debug(r);
	// Exception
	if (r->int_no < 32)
		fault(r);
	// Hardware Interrupt
	else if (r->int_no < 48)
		irq(r);
	// Software Interrupt
	else
		r->eax = syscall_process(r->eax, r->ebx, r->ecx, r->edx);
}