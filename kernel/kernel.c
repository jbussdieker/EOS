////////////////////////////////////////////////////////////////////////////////
// $Id: kernel.c,v 1.9 2010/12/28 02:23:08 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <kernel/syscall.h>
#include <kernel/fifo.h>
#include <kernel/tasks.h>
#include <stdio.h>
#include "../usr/console.h"
#include "../usr/gui.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
kernel_t kinfo;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
/// Called from arch to read the command line
/// \param cmd Address of command line string
void kernel_parse_cmdline(char *cmd)
{
	kinfo.args = cmd;
	
	while (*cmd != 0)
	{
		if (strncmp(cmd, "log=", 4) == 0)
			kinfo.loglevel = atoi(cmd+4);
		if (strncmp(cmd, "gui=", 4) == 0)
			kinfo.gui = atoi(cmd+4);
		if (strncmp(cmd, "headless", 8) == 0)
			kinfo.headless = 1;

		// Skip to next parameter
		while (isspace(*cmd) == 0)
			cmd++;
		while (isspace(*cmd) == 1)
			cmd++;
	}
}

/// Called from arch_init (not expected to return)
/// \param cmd Address of command line string
void kernel_init()
{
	printf("EOS Online\n\n");

	task_create(console_main, 0, "console", 0);
	
	//syscall(SYS_SLEEP, 0, 0, 0);
	
	//task_create(gui_main, 0, "gui", 0);

	// Visual check that the kernel thread is still running
	/*volatile unsigned char *idlespin = (unsigned char*)0xC00B809E;
	*idlespin++ = '@';
	while (1)
		*idlespin = *idlespin + 1;*/
}
