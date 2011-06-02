////////////////////////////////////////////////////////////////////////////////
// $Id: paging.c,v 1.5 2010/12/23 09:22:02 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <string.h>
#include <stdio.h>
#include <kernel/kernel.h>
#include <kernel/mm.h>

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
static void map_page(unsigned long *bpd, void *vaddr, void *paddr, int mode)
{
	unsigned long v = (unsigned long)vaddr;
	unsigned long p = (unsigned long)paddr;
	unsigned long *bpt;

	bpt = (void *)(bpd[v>>22] & 0xFFFFF000);
	if (bpt == 0)
	{
		bpt = (void *)PHYS_VIRT(mm_malloc());
		memset(bpt, 0, 4096);
		bpt[(v>>12) & 0x03FF] = p | 7;
		bpd[v>>22] = VIRT_PHYS((unsigned long)bpt) | mode;
	}
	else
	{
		bpt = (void *)PHYS_VIRT(bpt);
		bpt[(v>>12) & 0x03FF] = p | mode;		
	}
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void paging_map(unsigned long *bpd, void *vaddr, void *paddr, unsigned long size, int mode)
{
	int i;
	for (i = 0; i < (size / 4096); i++)
	{
		map_page(bpd, vaddr, paddr, mode);
		vaddr += 4096;
		paddr += 4096;
	}
}

void *paging_makeidentity()
{
	void *addr = (void *)PHYS_VIRT(mm_malloc());
	memset(addr, 0, 4096);
	paging_map(addr, (void *)0xC0000000, (void *)0x000000, 1024*1024*4, 7);
	paging_map(addr, (void *)0xC0400000, (void *)0x400000, 1024*1024*4, 7);
	return addr;
}

void paging_debug(unsigned long *pd)
{
	int i, j;
	unsigned long curvbase;
	unsigned long curpbase = 0;
	unsigned long size=123;
	
	for (i = 0; i < 1024; i++)
	{
		unsigned long *pt = (unsigned long *)(pd[i] & 0xFFFFF000);
		if (pt == 0)
			continue;
		
		pt = (unsigned long *)PHYS_VIRT(pt);
		
		curvbase = i << 22;
		
		for (j = 0; j < 1024; j++)
		{
			
			if ((pt[j] & 1) != 0)
			{
				// Did we jump offsets?
				if ((pt[j] & 0xFFFFF000) != (curpbase + 4096))
				{
					if (size != 123)
						printf("%dKB\n", size / 1024);
					printf("%08x-%08x ", curvbase, pt[j] & 0xFFFFF000);
					size = 0;
				}
				
				curpbase = pt[j] & 0xFFFFF000;
				//printf("%08x\n", pt[j]);
				size+=4096;
			}
			curvbase += 4096;
		}	
	}

	printf("%dKB\n", size / 1024);
}
