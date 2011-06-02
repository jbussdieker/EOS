////////////////////////////////////////////////////////////////////////////////
// $Id: fault.c,v 1.1 2010/12/18 00:58:26 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <kernel/tasks.h>
#include <kernel/elf.h>
#include "isr.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
static char *fault_strings[] =
{
	"Division By Zero",
	"Debug",
	"Non Maskable Interrupt",
	"Breakpoint Exception",
	"Into Detected Overflow Exception",
	"Out of Bounds Exception",
	"Invalid Opcode Exception",
	"No Coprocessor Exception",
	"Double Fault Exception",
	"Coprocessor Segment Overrun Exception",
	"Bad TSS Exception",
	"Segment Not Present Exception",
	"Stack Fault Exception",
	"General Protection Fault Exception",
	"Page Fault Exception",
	"Unknown Interrupt Exception",
	"Coprocessor Fault Exception",
	"Alignment Check Exception",
	"Machine Check Exception"
};

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
void fault(context_t *r)
{
	if (r->int_no <= 18)
		printf("%s\n", fault_strings[r->int_no]);
	else
		printf("Reserved\n");
	
	printf("    CS: 0x%08X\n", r->cs);
	printf("   EIP: 0x%08X\n\n", r->eip);
	printf("    DS: 0x%08X  ES: 0x%08X\n", r->ds, r->es);
	printf("    FS: 0x%08X  GS: 0x%08X\n\n", r->fs, r->gs);
	printf("   EAX: 0x%08X EBX: 0x%08X\n", r->eax, r->ebx);
	printf("   ECX: 0x%08X EDX: 0x%08X\n\n", r->ecx, r->edx);
	printf("   EDI: 0x%08X ESI: 0x%08X\n", r->edi, r->esi);
	printf("   EBP: 0x%08X ESP: 0x%08X\n\n", r->ebp, r->esp);
	printf("   Error Code: 0x%08X\n", r->err_code);
	
	// Check for page fault
	if (r->int_no == 14)
	{
		unsigned long cr2;
		__asm__("mov %%cr2, %0" : "=r" (cr2));
		printf("   Page Fault: 0x%08X\n", cr2);
	}
	printf("%s\n", elf_get_symbol((void *)r->cs));
	while(1);
	task_kill();
}
