////////////////////////////////////////////////////////////////////////////////
// $Id: tasks.c,v 1.3 2010/12/20 10:03:38 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/arch.h>
#include <kernel/tasks.h>
#include <kernel/elf.h>
#include <kernel/mm.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
static task_t idle = {	.kstack=0,
						.next=&idle,
						.esp=0,
						.name="idle",
						.req=0,
						.curpath="//",
						.node=0,
						.ticks=10,
						.priority=10};
						
static task_t *lasttask = &idle;
task_t *curtask = &idle;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
task_t *task_create(void *entry, int pl, char *name, int ticks)
{
	task_t *newtask;
	
	// Create memory space for a task, and 2 stacks one for user space and one for kernel space
	newtask = malloc(sizeof(task_t));

	strcpy(newtask->name, name);
	strcpy(newtask->curpath, "//");
	newtask->req = 0;
	newtask->node = devroot;
	newtask->ticks = newtask->priority = ticks;
	
	// Create a user and kernel stack
	newtask->stackptr = malloc(8192);
	void *newstack = newtask->stackptr + 8192;
	newtask->kstackptr = malloc(8192);
	void *newkstack = newtask->kstackptr + 8192;
	
	newtask->esp = arch_setup_context(newstack, pl, entry);

	void *newpde = paging_makeidentity();
	newtask->cr3 = (void *)VIRT_PHYS(newpde);
	
	// Used by task switcher
	newtask->kstack = newkstack;
	
	// Add to linked list
	newtask->next = curtask->next;
	curtask->next = newtask;

	return newtask;
}

void task_scheduler()
{
	lasttask = curtask;
	do {
		curtask = curtask->next;
	} while (curtask->req != 0);
}

void task_kill()
{
	task_t *killtask = lasttask->next;
	lasttask->next = lasttask->next->next;
	curtask = lasttask;
	free(killtask->stackptr);
	free(killtask->kstackptr);
	free(killtask);
	task_scheduler();
}

task_t *task_wake(int sleepint)
{
	task_t *t = &idle;

	do
	{
		if (t->req != 0)
			if (t->req->intnum == sleepint)
				return t;
		t = t->next;
	} while (t != &idle);

	return 0;
}

void task_debug()
{
	task_t *t = &idle;

	printf("Name     Stack    IP       CR3     Symbol\n");
	do
	{
		void *ip = arch_get_ip(t->esp);
		printf("%-8s %08X %08X %08X %s\n", t->name, t->esp, ip, t->cr3, elf_get_symbol(ip));
		t = t->next;
	} while (t != &idle);
}
