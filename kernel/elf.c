////////////////////////////////////////////////////////////////////////////////
// $Id: elf.c,v 1.3 2010/12/20 08:49:14 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <kernel/paging.h>
#include <kernel/tasks.h>
#include <kernel/mm.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../arch/multiboot.h"

typedef struct elf32_header
{
	unsigned char ident[16];
	unsigned short type;
	unsigned short machine;
	unsigned long version;
	unsigned long entry;
	unsigned long phoff;
	unsigned long shoff;
	unsigned long flags;
	unsigned short ehsize;
	unsigned short phentsize;
	unsigned short phnum;
	unsigned short shentsize;
	unsigned short shnum;
	unsigned short shstrndx;
} elf32_header;

typedef struct elf32_pheader
{
	unsigned long type;
	unsigned long offset;
	unsigned long vaddr;
	unsigned long paddr;
	unsigned long filesz;
	unsigned long memsz;
	unsigned long flags;
	unsigned long align;
} elf32_pheader;

typedef struct elf32_sheader
{
	unsigned long name;
	unsigned long type;
	unsigned long flags;
	unsigned long addr;
	unsigned long offset;
	unsigned long size;
	unsigned long link;
	unsigned long info;
	unsigned long addralign;
	unsigned long entsize;
} elf32_sheader;

typedef struct elf32_sym
{
	unsigned long name;
	unsigned long addr;
	unsigned long size;
	unsigned char info;
	unsigned char other;
	unsigned short shndx;
} elf32_sym;



#define ET_NONE		0
#define ET_REL		1
#define ET_EXEC		2
#define ET_DYN		3
#define ET_CORE		4
#define ET_LOPROC	0xFF00
#define ET_HIPROC	0xFFFF

#define EM_NONE		0
#define EM_M32		1
#define EM_SPARC	2
#define EM_386		3
#define EM_68K		4
#define EM_88K		5
#define EM_860		7
#define EM_MIPS		8

#define PT_NULL		0
#define PT_LOAD		1
#define PT_DYNAMIC	2
#define PT_INTERP	2
#define PT_NOTE		2
#define PT_SHLIB	2
#define PT_PHDR		2
#define PT_LOPROC	0x70000000
#define PT_HIPROC	0x7FFFFFFF



/////// KERNEL ELF
multiboot_elf_section_header_table_t *meh;
elf32_sheader *sh;

char *strings;
elf32_sym *symbols;
unsigned long symbolcount = 0;
char *strtab;

void elf_init_kernel(void *p)
{
	kprintf(1, "elf_init_kernel(p: %08X)\n", p);

	meh = p;
	elf32_sheader *strheader = (void *)((PHYS_VIRT(meh->addr) + (meh->size * meh->shndx)));
	strings = (void *)PHYS_VIRT(strheader->addr);
	
	int i;
	kprintf(2, "%d sections\n", meh->num);
	for (i = 1; i < meh->num; i++)
	{
		elf32_sheader *curheader = (void *)((PHYS_VIRT(meh->addr) + (meh->size * i)));
		
		kprintf(3, "section %d name: %s size: %d\n", i, strings+curheader->name, curheader->size);
		if (curheader->type == 3)
			strtab = (void *)PHYS_VIRT(curheader->addr);
		if (curheader->type == 2)
		{
			symbols = (void *)PHYS_VIRT(curheader->addr);
			symbolcount = curheader->size / sizeof(elf32_sym);
		}

		/*printf("name: %s ", strings+curheader->name);
		printf("type: %d ", curheader->type);
		printf("align: %d ", curheader->addralign);
		printf("flags: %d ", curheader->flags);
		printf("addr: %08x ", curheader->addr);
		printf("size: %08x ", curheader->size);
		printf("\n");*/
	}
}

void *elf_get_symbol_addr(char *symbol)
{
	int j;
	
	for (j = 0; j < symbolcount; j++)
		if (symbols[j].shndx == 1)
			if (strcmp(symbol, strtab+symbols[j].name) == 0)
				return (void *)symbols[j].addr;
	
	return 0;
}

char *elf_get_symbol(void *p)
{
	int j;
	char *curname = "?";
	void *cur = 0;
	
	for (j = 0; j < symbolcount; j++)
	{
		if (symbols[j].shndx == 1)
		{
			if (((void *)symbols[j].addr > cur) && ((void *)symbols[j].addr < p))
			{
				curname = strtab+symbols[j].name;
				cur = (void *)symbols[j].addr;
			}
			
			//printf("name: %s ", strtab+symbols[j].name);
			//printf("addr: %08x\n", symbols[j].addr);
			//printf("size: %08x ", symbols[j].size);
			//printf("info: %02x ", symbols[j].info);
			//printf("other: %02x ", symbols[j].other);
			//printf("shndx: %04x\n", symbols[j].shndx);
		}
	}
	
	return curname;
}
/////// END KERNEL ELF



/////// DRIVERS ELF

void elf_load_driver(void *p)
{
	elf32_header *eh = p;

	kprintf(1, "elf_load_driver started %08X\n", p);
	
	assert(eh->type == ET_REL);
	assert(eh->machine == EM_386);
	assert(eh->version == 1);

	unsigned long entry = VIRT_PHYS(eh->entry);
	unsigned long physaddr = 0, virtaddr;

	int i;
	for (i = 0; i < eh->phnum; i++)
	{
		elf32_pheader *ph = ((p + eh->phoff) + (eh->phentsize * i));
		
		// Program type
		if (ph->type != PT_LOAD)
			kprintf(0, "elf_init: invalid program type\n", 0);

		physaddr = VIRT_PHYS(p + ph->offset);
		virtaddr = ph->vaddr;
	}
	
	kprintf(1, "elf_load_driver success entry: %08X phys: %08X\n", entry, physaddr);
	
	//task_t *t = task_create(entry, 3, "elf", "test");
	
	//paging_map(PHYS_VIRT(t->cr3), virtaddr, physaddr, 1);
}

/////// END DRIVERS ELF

