////////////////////////////////////////////////////////////////////////////////
// $Id: serial.c,v 1.8 2010/12/18 01:18:00 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <kernel/kernel.h>
#include <kernel/arch.h>
#include <kernel/fs.h>
#include <kernel/dev.h>

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
typedef struct serial_handle_t
{
	fs_node_t fs;
	int base;
} serial_handle_t;

////////////////////////////////////////////////////////////////////////////////
// Local Variables
////////////////////////////////////////////////////////////////////////////////
unsigned char *buffer;

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
void serial_isr(void *c)
{
	printf("serial_isr\n");
}

static void serial_putch(serial_handle_t *h, unsigned char c)
{
	if (c == '\n')
		outportb(h->base, '\r');
	
	outportb(h->base, c);
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
static unsigned int serial_read(void *sh, void *buffer, size_t size)
{
	serial_handle_t *s = (serial_handle_t *)sh;

	int i;
	unsigned char *writeptr = (unsigned char *)buffer;
	for (i = 0; i < size; i++)
	{
		while ((inportb(s->base + 5) & 0x1) == 0);
		*writeptr = inportb(s->base);
		if (*writeptr == '\r')
		{
			i--;
			continue;
		}
		writeptr++;
	}
	
	return size;
}

static unsigned int serial_write(void *p, void *buffer, size_t size)
{
	serial_handle_t *h = (serial_handle_t *)p;
	
	int i;
	unsigned char *writeptr = (unsigned char *)buffer;
	for (i = 0; i < size; i++)
	{
		while ((inportb(h->base + 5) & 0x20) == 0);
		serial_putch(h, *writeptr);
		writeptr++;
	}
	
	return size;
}

fs_node_t *serial_init(unsigned short port)
{
	kprintf(1, "serial_init(port: 0x%X)\n", port);

	// Create new handle
	serial_handle_t *dev;
	dev = (serial_handle_t *)dev_alloc("serial", sizeof(serial_handle_t));
	
	// Fill universal fields
	dev->fs.read = serial_read;
	dev->fs.write = serial_write;
	dev->fs.blocksize = 1;
	
	// Fill custom fields
	dev->base = port;

	// Custom initialization
	outportb(dev->base + 1, 0x00);    // Disable all interrupts
	outportb(dev->base + 3, 0x80);    // Enable DLAB (set baud rate divisor)
	outportb(dev->base + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
	outportb(dev->base + 1, 0x00);    //                  (hi byte)
	outportb(dev->base + 3, 0x03);    // 8 bits, no parity, one stop bit
	outportb(dev->base + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
	outportb(dev->base + 4, 0x0B);    // IRQs enabled, RTS/DSR set

	arch_register_irq(6, serial_isr);
	buffer = malloc(512);

	return (fs_node_t *)dev;
}
