////////////////////////////////////////////////////////////////////////////////
// $Id: dev.c,v 1.8 2010/12/28 02:23:08 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <kernel/kernel.h>
#include <kernel/dev.h>
#include <kernel/fs.h>
#include <kernel/tasks.h>
#include "../drivers/null.h"
#include "../drivers/ramfs.h"
#include "../drivers/keyb.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/tty.h"
#include "../drivers/mouse.h"

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
fs_node_t *devroot = 0;
fs_node_t *vgadev = 0;
fs_node_t *nulldev = 0;
fs_node_t *keybdev = 0;
fs_node_t *ttydev = 0;
fs_node_t *serialdev = 0;
fs_node_t *mousedev = 0;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
/// Initialize all device drivers
void dev_init()
{
	kprintf(1, "dev_init()\n", 0);
	
	// This must be the first device initialized
	fsroot    = ramfs_init();

	// Initialize BUSes
	pci_init();
	usb_init();
	
	// Initialize Simple Devices
	nulldev   = null_init();
	serialdev = serial_init(0x3F8);
	ttydev    = tty_init(0xC00B80A0, 80, 24, 0x1F);
	mousedev  = mouse_init(0x60);
	keybdev   = keyb_init(0x60);
	vgadev    = vga_init();

	// Setup default stdin/stdout mappings
	if (kinfo.headless == 1)
	{
		stdin = (void *)&serialdev;
		stdout = stderr = (void *)&serialdev;
	}
	else
	{
		stdin = (void *)&keybdev;
		stdout = stderr = (void *)&ttydev;
	}
}

fs_node_t *dev_alloc(char *name, int size)
{
	fs_node_t *node;
	node = malloc(size);
	memset(node, 0, size);
	strcpy(node->name, name);
	if (devroot->first_child == 0)
	{
		devroot->first_child = node;
		node->prev = 0;
		node->next =0;
	}
	else
	{
		fs_node_t *addbefore = devroot->first_child;
		node->prev = 0;
		node->next = addbefore;
		addbefore->prev = node;
		devroot->first_child = node;
	}
		
	return node;
}
