////////////////////////////////////////////////////////////////////////////////
// $Id: mouse.c,v 1.12 2010/12/23 10:31:38 Ecco Exp $
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
#include <assert.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
typedef struct mouse_handle_t
{
	fs_node_t fs;
	int port;
	int shift;
	fifo_buffer_t *buffer;
} mouse_handle_t;

mouse_handle_t *dev;

////////////////////////////////////////////////////////////////////////////////
// Static Functions
////////////////////////////////////////////////////////////////////////////////
void mouse_irq(void *r)
{
	unsigned char val = inportb(dev->port);
	fifo_put(dev->buffer, val);

	task_t *reqtask;
	reqtask = task_wake(12);
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

static void writecmdbyte(mouse_handle_t *dev, unsigned char cmd)
{
	while (inportb(dev->port+4) & 2);
	outportb(dev->port+4, cmd);
}

static void writebyte(mouse_handle_t *dev, unsigned char byte)
{
	while (inportb(dev->port+4) & 2);
	outportb(dev->port, byte);
}

static unsigned char readbyte(mouse_handle_t *dev)
{
	unsigned char r;
	while ((inportb(dev->port+4) & 1) == 0);
	r = inportb(dev->port);
	return r;
}

static void writemousecmdbyte(mouse_handle_t *dev, unsigned char cmd)
{
	writecmdbyte(dev, 0xD4);
	writebyte(dev, cmd);
}

void setsamplerate(mouse_handle_t *dev, unsigned char rate)
{
	writemousecmdbyte(dev, 0xF3);
	assert(readbyte(dev) == 0xFA);
	writemousecmdbyte(dev, rate);
	assert(readbyte(dev) == 0xFA);
}

static void setscaling(mouse_handle_t *dev, unsigned char scale)
{
	if (scale == 1)
		writemousecmdbyte(dev, 0xE6);
	if (scale == 2)
		writemousecmdbyte(dev, 0xE7);

	assert(readbyte(dev) == 0xFA);
}

static void enablereporting(mouse_handle_t *dev)
{
	writemousecmdbyte(dev, 0xF4);
	assert(readbyte(dev) == 0xFA);
}

static void enablestream(mouse_handle_t *dev)
{
	writemousecmdbyte(dev, 0xEA);
	assert(readbyte(dev) == 0xFA);
}

static void setresolution(mouse_handle_t *dev, unsigned char resolution)
{
	writemousecmdbyte(dev, 0xE8);
	assert(readbyte(dev) == 0xFA);
	writemousecmdbyte(dev, resolution);
	assert(readbyte(dev) == 0xFA);
}

static unsigned char getid(mouse_handle_t *dev)
{
	writemousecmdbyte(dev, 0xF2);
	unsigned char ret;
	ret = readbyte(dev);
	if (ret != 0xFA)
		return 0xFF;
	assert(ret == 0xFA);
	unsigned char id = readbyte(dev);
	return id;
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
static unsigned int mouse_read(void *p, void *buffer, size_t size)
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
	req->intnum = 12;
	curtask->req = req;
	task_scheduler();
	return size;
}

fs_node_t *mouse_init(unsigned short port)
{
	kprintf(1, "mouse_init(port: 0x%X)\n", port);

	// Create new handle
	dev = (mouse_handle_t *)dev_alloc("mouse", sizeof(mouse_handle_t));

	// Fill universal fields
	dev->fs.read = mouse_read;
	dev->fs.blocksize = 1;

	// Fill custom fields
	dev->port = port;
	dev->buffer = fifo_alloc(128);

	unsigned char status;
	
	// Read status byte command
	writecmdbyte(dev, 0x20);
	status = readbyte(dev);

	// Unset 0x20
	status = (status & ~0x20);
	// Set bit 1
	status |= 3;
	
	// Write status byte command
	writecmdbyte(dev, 0x60);
	writebyte(dev, status);

	// Read status again
	writecmdbyte(dev, 0x20);
	status = readbyte(dev);

	// HACK for real computer to skip mouse
	if (getid(dev) == 0xFF)
		return 0;

	setsamplerate(dev, 200);
	setsamplerate(dev, 100);
	setsamplerate(dev, 80);

	setsamplerate(dev, 200);
	setsamplerate(dev, 200);
	setsamplerate(dev, 80);
	
	if (getid(dev) == 3)
	{
		kprintf(2, "found scrollwheel mouse\n", 0);
	}
	else if (getid(dev) == 2)
	{
		kprintf(2, "found 5-button mouse\n", 0);
	}
	else
	{
		kprintf(2, "found standard mouse\n", 0);
	}

	setsamplerate(dev, 200);
	setresolution(dev, 1);
	setscaling(dev, 2);
	enablereporting(dev);
	enablestream(dev);
	
	arch_register_irq(12, mouse_irq);
	
	return (fs_node_t *)dev;
}
