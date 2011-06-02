////////////////////////////////////////////////////////////////////////////////
// $Id: keyb.c,v 1.15 2010/12/28 01:27:31 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <kernel/fifo.h>
#include <kernel/arch.h>
#include <kernel/syscall.h>
#include <kernel/tasks.h>
#include <kernel/dev.h>
#include <kernel/fs.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Static variables
////////////////////////////////////////////////////////////////////////////////
static char keyb[] = {0x0,  0x0, '1', '2', '3',  '4', '5',  '6', 
					  '7',  '8', '9', '0', '-',  '=', '\b', '\t',
					  'q',  'w', 'e', 'r', 't',  'y', 'u',  'i', 
					  'o',  'p', '[', ']', '\n', 0x0, 'a',  's',
					  'd',  'f', 'g', 'h', 'j',  'k', 'l',  ';', 
					  '\'', '`', 0x0, 0x0, 'z',  'x', 'c',  'v', 
					  'b',  'n', 'm', ',', '.',  '/', 0x0,  '*', 
					  0x0,  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  '7', 
					  '8',  '9', '-', '4', '5',  '6', '+',  '1', 
					  '2',  '3', '0', '.', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' '};

static char keybu[] ={0x0,  0x0, '!', '@', '#',  '$', '%',  '^', 
					  '&',  '*', '(', ')', '_',  '+', '\b', '\t',
					  'Q',  'W', 'E', 'R', 'T',  'Y', 'U',  'I', 
					  'O',  'P', '{', '}', '\n', 0x0, 'A',  'S',
					  'D',  'F', 'G', 'H', 'J',  'K', 'L',  ':', 
					  '|', '~', 0x0, 0x0, 'Z',  'X', 'C',  'V', 
					  'B',  'N', 'M', '<', '>',  '?', 0x0,  '*', 
					  0x0,  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  '7', 
					  '8',  '9', '-', '4', '5',  '6', '+',  '1', 
					  '2',  '3', '0', '.', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' ', 
					  ' ',  ' ', ' ', ' ', ' ',  ' ', ' ',  ' '};

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
typedef struct keyb_handle_t
{
	fs_node_t fs;
	int port;
	int shift;
	int make;
	fifo_buffer_t *buffer;
} keyb_handle_t;

static keyb_handle_t *dev;

////////////////////////////////////////////////////////////////////////////////
// Static Functions
////////////////////////////////////////////////////////////////////////////////
static unsigned int keyb_read(void *p, void *buffer, size_t size);

void keyb_irq(void *r)
{
	unsigned char val = inportb(dev->port);
	char c = val & 0x7F;
	
	//printf("%02X\n", c);
	
	// HW specific make code for special keys
	if (dev->make == 0x60)
	{
		if (val == 0x51)
			tty_scrolldown();
		if (val == 0x49)
			tty_scrollup();
		/*if (val == 0x48)
			console_scrollup();
		if (val == 0x50)
			console_scrolldown();*/
			
		dev->make = 0;
		return;
	}
	if (c == 0x60)
	{
		dev->make = 0x60;
		return;
	}
	
	// F1
	if (c == 0x3B)
	{
		kinfo.gui = 0;
		return;
	}
	// F2
	if (c == 0x3C)
	{
		kinfo.gui = 1;
		return;
	}

	// LShift RShift
	if ((c == 0x2a) || (c == 0x36))
	{
		if (val & 0x80)
			dev->shift = 0;
		else
			dev->shift = 1;
		return;
	}

	// Keypress
	if (val & 0x80)
	{
		return;
	}
	// Keyrelease
	else
	{
		if (dev->shift == 1 && keybu[val] != 0)
			fifo_put(dev->buffer, keybu[val]);
		else if (keyb[val] != 0)
			fifo_put(dev->buffer, keyb[val]);
	}

	task_t *reqtask;
	reqtask = task_wake(1);
	if (reqtask == 0)
		return;	
	ioreq_t *req = reqtask->req;

	if (fifo_getsize(dev->buffer) >= req->size)
	{
		fifo_get(dev->buffer, req->buffer, req->size);
		free(req);
		reqtask->req = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
static unsigned int keyb_read(void *p, void *buffer, size_t size)
{
	if (fifo_getsize(dev->buffer) >= size)
	{
		fifo_get(dev->buffer, buffer, size);
		return size;
	}

	ioreq_t *req;
	req = malloc(sizeof(ioreq_t));
	req->buffer = buffer;
	req->size = size;
	req->intnum = 1;	
	curtask->req = req;
	task_scheduler();
	return size;
}

fs_node_t *keyb_init(unsigned short port)
{
	kprintf(1, "keyb_init(port: 0x%X)\n", port);

	// Create new handle
	dev = (keyb_handle_t *)dev_alloc("keyb", sizeof(keyb_handle_t));

	// Fill universal fields
	dev->fs.read = keyb_read;
	dev->fs.blocksize = 1;

	// Fill custom fields
	dev->port = port;
	dev->buffer = fifo_alloc(32);
	dev->make = 0;
	dev->shift = 0;

	arch_register_irq(1, keyb_irq);

	return (fs_node_t *)dev;
}
