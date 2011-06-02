////////////////////////////////////////////////////////////////////////////////
// $Id: mm.c,v 1.3 2010/12/23 09:22:02 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <kernel/tasks.h>
#include <kernel/mm.h>
#include <assert.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define MM_PAGE_SIZE 4096

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
typedef struct mem_stub
{
	void *ptr;
	unsigned long pagecount;
	unsigned long reqsize;
	task_t *task;
	struct mem_stub *next;
	struct mem_stub *prev;
} mem_stub;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
static unsigned char *memmap = 0;
static mem_stub mem_stub_root = {.ptr=0,.pagecount=0,.reqsize=0,.task=0,.next=&mem_stub_root,.prev=&mem_stub_root};
static mem_stub *msroot = &mem_stub_root;
static int mmready = 0;

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
void mm_map(void *base, unsigned long size, unsigned int mask)
{
	kprintf(4, "mm_map base: %08X size: %d mask: %d\n", base, size, mask);

	// Allocator not initialized error
	if (mmready == 0)
		return;

	unsigned long i;
	unsigned long ptr = (unsigned long)base;
	
	for (i = 0; i < ((size + MM_PAGE_SIZE - 1) / MM_PAGE_SIZE); i++)
	{
		// Get byte
		unsigned char val = memmap[ptr / (MM_PAGE_SIZE * 8)];

		// Set/Unset bit
		unsigned char shift = (ptr / MM_PAGE_SIZE) % 8;
		if (mask == 1)
			val = (val | (0x80 >> shift));
		else
			val = (val & ~(0x80 >> shift));

		// Return byte
		memmap[ptr / (MM_PAGE_SIZE * 8)] = val;
		
		// Next block
		ptr += MM_PAGE_SIZE;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void mm_init(unsigned long size)
{
	kprintf(1, "mm_init(size: %d)\n", size);
	
	memmap = kinfo.end;
	memset(memmap, 0x00, size);
	
	// We can begin allocations at this point
	mmready = 1;
	
	// Mark the kernel, initfs, drivers, and memmap as used
	kinfo.end += 128 * 1024;
	mm_map((void *)VIRT_PHYS(kinfo.start), kinfo.end - kinfo.start, 1);
}

void *mm_malloc()
{
	// Find a free page and return it (start at 1MB)
	int i, j;
	for (i = 32; i < (1024*128); i++)
	{
		if (memmap[i] != 0xFF)
		{
			unsigned long base = i * MM_PAGE_SIZE * 8;		
			unsigned char val = memmap[i];
			for (j = 0; j < 8; j++)
			{
				if (~val & 0x80)
				{
					mm_map((void *)base, MM_PAGE_SIZE, 1);	
					return (void *)base;
				}

				val = val << 1;
				base += MM_PAGE_SIZE;
			}
		}
	}
	return 0;
}

void *malloc(size_t reqsize)
{
	kprintf(4, "malloc size: %d\n", reqsize);
	assert(mmready == 1);
	assert(reqsize > 0);

	int size = reqsize;

	mem_stub *ms = (mem_stub *)PHYS_VIRT(mm_malloc());

	// Add to the linked list					
	ms->prev = msroot;
	ms->next = msroot->next;
	ms->next->prev = ms;
	msroot->next = ms;
	
	// Fill struct
	ms->reqsize = reqsize;
	ms->pagecount = (size + MM_PAGE_SIZE - 1) / MM_PAGE_SIZE;
	ms->task = curtask;
	
	// Find a free page and return it (start at 1MB)
	int freecount = 0;
	unsigned long base = 0;
	
	int i,j;
	for (i = 32; i < (1024*128); i++)
	{
		unsigned char cur;
		cur = memmap[i];

		if (memmap[i] != 0xFF)
		{
			for (j = 0; j < 8; j++)
			{
				if (~cur & 0x80)
				{
					freecount++;
					
					// Calculate the pointer base if this is the first page
					if (freecount == 1)
						base = (i * MM_PAGE_SIZE * 8) + (j * MM_PAGE_SIZE);
	
					// If we have enough free pages then finish the struct and return
					if (freecount == ms->pagecount)
					{
						mm_map((void *)base, freecount * MM_PAGE_SIZE, 1);
						ms->ptr = (void *)base;
						
						//memset(PHYS_VIRT(ms->ptr), 0, ms->pagecount * MM_PAGE_SIZE);
						
						return (void *)PHYS_VIRT(ms->ptr);
					}
				}
				else
					freecount = 0;

				cur = cur << 1;
			}
		}
	}
	
	return 0;
}

void free(void *p)
{
	kprintf(4, "free ptr: %08X\n", p);
	assert(mmready == 1);
	assert(p != 0);

	mem_stub *ms = msroot;
	do
	{
		if (ms->ptr == (void *)VIRT_PHYS(p))
		{
			ms->prev->next = ms->next;
			ms->next->prev = ms->prev;
			
			mm_map(ms->ptr, ms->pagecount * MM_PAGE_SIZE, 0);
			mm_map((void *)VIRT_PHYS(ms), MM_PAGE_SIZE, 0);
			return;
		}
		ms = ms->next;
	}
	while (ms != msroot);
	assert(0);
}

unsigned long mm_used()
{
	assert(mmready == 1);
	
	unsigned long ret = 0;
	mem_stub *ms = msroot;
	do
	{
		ret += ms->pagecount * MM_PAGE_SIZE;
		ms = ms->next;
	} while (ms != msroot);
	
	return ret;
}
