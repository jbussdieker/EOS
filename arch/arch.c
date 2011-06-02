////////////////////////////////////////////////////////////////////////////////
// $Id: i386.c,v 1.20 2010/12/28 02:23:07 Ecco Exp $
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
#include <kernel/elf.h>
#include <kernel/tasks.h>
#include <kernel/mm.h>
#include <kernel/dev.h>
#include <kernel/syscall.h>
#include <assert.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "../drivers/tty.h"
#include "multiboot.h"
#include "isr.h"
#include "cpu.h"
#include "pic.h"
#include "pit.h"

extern int _ld_kernel_start, _ld_kernel_end;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void *arch_get_ip(void *p)
{
	context_t *c;
	
	c = (context_t *)p;
	return c->eip;
}

/// Create an initial task context
/// \param p Pointer to the destination stack
/// \param pl Privledge level of task
/// \param entry Address of first instruction
void *arch_setup_context(void *p, int pl, void *entry)
{
	context_t *c;
	c = (context_t *)(p-sizeof(context_t)-8);
	memset(c, 0, sizeof(c));
	
	if (pl == 0)
	{
		*((unsigned long *)(p-8)) = 0;
		*((unsigned long *)(p-12)) = 0;
	}
	else
	{
		*((unsigned long *)(p-4)) = 0;
		*((unsigned long *)(p-8)) = 0;
	}
	
	// For PL3 tasks we also need the SS and ESP
	if (pl != 0)
		c->uss = 0x23;
	c->uesp = p-12; // 3 Things left the SS and both args
	// The following will be used for all task switches
	c->eflags = 0x0202;
	if (pl == 0)
		c->cs = 0x08;
	else
		c->cs = 0x1B;
		
	c->eip = entry;
	if (pl == 0)
	{
		c->ds = 0x10;
		c->es = 0x10;
		c->fs = 0x10;
		c->gs = 0x10;
	}
	else
	{
		c->ds = 0x23;
		c->es = 0x23;
		c->fs = 0x23;
		c->gs = 0x23;
	}
	
	return (void *)c;
}

/// Load Grub module drivers
/// \param mod Virtual address of Grub multiboot module info structure (*? bytes)
/// \param count Number of modules/drivers to load (4 bytes)
void arch_init_drivers(multiboot_module_t *mod, int count)
{
	int i;
	//drivers_start = PHYS_VIRT(mod[0].mod_start);
	for (i = 0; i < count; i++)
		printf("loading %s\n", PHYS_VIRT(mod[i].cmdline));
	//drivers_end = PHYS_VIRT(mod[i-1].mod_end);
}

/// Called from our bootstub
/// \param mbd Virtual address of Grub multiboot info structure (*? bytes)
/// \param magic Grub boot signature (0x1BADB002) (4 bytes)
void arch_init(multiboot_info_t* mbd, unsigned int magic)
{
	// Create the kernel info build string
	sprintf(kinfo.buildstring, "EOS (Built %s %s using GNU C version %s)", __DATE__, __TIME__, __VERSION__);

	// Low level hardware setup of the interrupt controller and system timer
	pic_init();
	pit_init(100);
	asm ("sti");

	// Clear screen
	tty_clear();
	
	// Parse the command line
	kernel_parse_cmdline((char *)PHYS_VIRT(mbd->cmdline));

	// Now safe to print to the kernel log
	printf("%s\n", kinfo.buildstring);
	kprintf(1, "arch_init(mbd: %08X, magic: %08X)\n", mbd, magic);

	// Read CPU information
	cpu_init();
	
	// Read placement information
	kinfo.start = &_ld_kernel_start;
	kinfo.end = &_ld_kernel_end;
	kinfo.memory = mbd->mem_upper;

	// Read kernel elf headers
	elf_init_kernel(&mbd->u.elf_sec);

	// Read module information
	if (mbd->mods_count > 0)
	{
		kprintf(2, "found %d modules\n", mbd->mods_count);

		// Read multiboot module information
		multiboot_module_t *mod;
		mod = (void *)PHYS_VIRT(mbd->mods_addr);
		
		// Expand kernel end to include the loaded modules
		kinfo.end = (void *)PHYS_VIRT(mod[mbd->mods_count - 1].mod_end);
		
		// The first module is the initfs the rest are drivers
		if (mbd->mods_count > 1)
			arch_init_drivers(&mod[1], mbd->mods_count - 1);
	}
	
	// Start the memory manager
	mm_init(128 * 1024);
	
	dev_init();
	kernel_init();
}
