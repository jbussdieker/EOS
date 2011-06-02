////////////////////////////////////////////////////////////////////////////////
// $Id: console.c,v 1.19 2010/12/28 00:48:52 Ecco Exp $
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
#include <kernel/tasks.h>
#include <kernel/paging.h>
#include <kernel/elf.h>
#include <kernel/mm.h>
#include <kernel/fs.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include "../drivers/pci.h"
#include "../arch/cpu.h"
#include "console.h"
#include "gui.h"

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
//#define printf uprintf
#define getchar ugetchar

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
int ugetchar(void)
{
	char c;
	extern fs_node_t *keybdev;
	syscall(SYS_READ, (unsigned long)keybdev, (unsigned long)&c, 1);
	return c;
}

/*int uprintf(const char *frmt, ...)
{
	char finalbuffer[128];

	void *args;
	va_start(args, frmt);
	vsprintf(finalbuffer, frmt, args);	
	va_end(args);
 
	return syscall(SYS_WRITE, stdout, finalbuffer, 1);
}*/

void *open(char *name)
{
	return (void *)syscall(SYS_OPEN, (unsigned long)name, 0, 0);
}

int read(void *h, void *b, unsigned long size)
{
	return syscall(SYS_READ, (unsigned long)h, (unsigned long)b, size);
}

int write(void *h, void *b, unsigned long size)
{
	return syscall(SYS_WRITE, (unsigned long)h, (unsigned long)b, size);
}

void seek(void *h, unsigned long o)
{
	syscall(SYS_SEEK, (unsigned long)h, o, 0);
}

void close(void *h)
{
	syscall(SYS_CLOSE, (unsigned long)h, 0, 0);
}

struct dirent *readdir(void *d)
{
	return (struct dirent *)syscall(SYS_READDIR, (unsigned long)d, 0, 0);
}

void console_cd(char *name)
{
	syscall(SYS_CD, (unsigned long)name, 0, 0);
}

void console_dir()
{
	DIR *d = open(0);
	struct dirent *de;
	int i = 0;
	
	if (d == NULL)
	{
		printf("Error opening current directory\n");
		return;
	}

	while ((de = readdir(d)) != NULL)
	{
		i++;
		printf(" %s\n", de->name);
	}
	
	printf("%d files\n", i);

	close(d);
}

void console_inb(char *c)
{
	if (*c != 0)
	{
		unsigned long port;
		port = atoi(c);
		printf("0x%x %02x\n", port, inportb(port));
		return;
	}

	printf("Usage:\n inb [port]\n\n port = 16-bit port address\n\n");
}

void console_outb(char *c)
{
	if (*c != 0)
	{
		unsigned long port;
		unsigned long value;
		port = atoi(c);
		while (isspace(*c) == 0)
			c++;
		while (isspace(*c) == 1)
			c++;
		value = atoi(c);
		outportb(port, value);
		printf("out %d value %d\n", port, value);
		return;
	}

	printf("Usage:\n outb [port] [value]\n\n port = 16-bit port address\n value = 8-bit value\n\n");
}

void mem(void *ptr)
{
	paging_map((void *)PHYS_VIRT(curtask->cr3), ptr, ptr, 4096, 7);
	printf("%08x = %08x\n", ptr, *((unsigned long *)ptr));
}

void *curhandle = 0;

void console_read(char *c)
{
	void *buffer;
	int size = atoi(c);
	buffer = malloc(4096);

	read(curhandle, buffer, size);

	int i, j; 
	for (i = 0; i < 32; i++)
	{
		unsigned char *hexedit = buffer + i * 16;
		
		for (j = 0; j < 16; j++)
		{
			printf("%02x ", *hexedit);
			if (i*16+j>=size-1)
				break;			
			hexedit++;
		}

		hexedit = buffer + i * 16;

		for (j = 0; j < 16; j++)
		{
			if (*hexedit > 31)
				printf("%c", *hexedit);
			else
				printf(".");

			if (i*16+j>=size-1)
				break;			
			hexedit++;
		}
		printf("\n");

		if (i*16+j>=size-1)
			break;			
	}

	free(buffer);
}

void string(void *p)
{
	printf("%s\n", p);
}

void console_cmd(char *c)
{
	char *args = strchr(c, ' ');
	int argc = 0;

	if (args != NULL)
	{
		*args = 0;
		args++;
	}
	void *(*func)() = elf_get_symbol_addr(c);

	////////////////////////////////
	// Lookup kernel symbol
	////////////////////////////////
	if (func != NULL)
	{
		void *ret = 0;

		if (args != NULL)
		{
			void *parsed_args[4];
			void *(*func_args1)(void *) = func;
			void *(*func_args2)(void *, void *) = func;
			void *(*func_args3)(void *, void *, void *) = func;
			
			while (1)
			{
				parsed_args[argc] = atoi(args);
				argc++;
				
				while(isxdigit(*args) || *args == 'x')
					args++;
				if (*args == 0)
					break;
				while(isxdigit(*args) == 0)
					args++;
			}			

			if (argc == 1)
				ret = func_args1(parsed_args[0]);
			else if (argc == 2)
				ret = func_args2(parsed_args[0], parsed_args[1]);						
			else if (argc == 3)
				ret = func_args3(parsed_args[0], parsed_args[1], parsed_args[2]);						
		}
		else
			ret = func();

		printf("Returned %08X\n", ret);
	}
	////////////////////////////////
	// Canned commands
	////////////////////////////////
	else if (strcmp(c, "pde") == 0)
		paging_debug((void *)PHYS_VIRT(curtask->cr3));
	else if (strncmp(c, "inb ", 4) == 0)
		console_inb(c+4);
	else if (strncmp(c, "outb ", 5) == 0)
		console_outb(c+5);
	else if (strcmp(c, "ticks") == 0)
		printf("%d ticks\n", kinfo.ticks);
	else if (strncmp(c, "cd ", 3) == 0)
		console_cd(c+3);
	else if (strncmp(c, "open ", 5) == 0)
		printf("handle: %08x\n", curhandle = open(c+5));
	else if (strcmp(c, "close") == 0)
		close(curhandle);
	else if (strncmp(c, "read ", 5) == 0)
		console_read(c+5);
	else if (strncmp(c, "write ", 6) == 0)
		write(curhandle, c+6, strlen(c+6));
	else if (strncmp(c, "seek ", 5) == 0)
		seek(curhandle, atoi(c+5));
	else
		printf("Invalid Command %s\n", c);
}

void console_main(int argc, char **argv)
{	
	//printf("\x1B[34mConsole Ready\n> ");
	printf("Console Ready\n> ");		

	char lastcommand[32];
	char command[32];
	int i = 0;
	
	while (1)
	{
		command[i] = 0;

		char c;
		c = getchar();
		
		if (c == '\n')
		{
			printf("\n", command);
			if (i > 0)
			{
				strcpy(lastcommand, command);
				console_cmd(command);
			}
			i = 0;
			printf("> ");
			continue;
		}
		if (c == '\t')
		{
			printf("%s", lastcommand);
			strcpy(command, lastcommand);
			i = strlen(command);
		}
		else if (c == '\b')
		{
			if (i > 0)
			{
				i--;
				command[i] = 0;
				printf("%c", c);
			}
		}
		else
		{
			command[i] = c;
			i++;
			printf("%c", c);
		}
	}
}
