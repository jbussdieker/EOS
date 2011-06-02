////////////////////////////////////////////////////////////////////////////////
// $Id: fifo.c,v 1.1 2010/12/20 10:05:17 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/fifo.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
fifo_buffer_t *fifo_alloc(int size)
{
	fifo_buffer_t *f;
	f = malloc(sizeof(fifo_buffer_t));
	f->size = size+1;
	f->buffer = malloc(size+1);
	f->out = f->in = f->buffer;
	return f;
}

unsigned long fifo_getsize(fifo_buffer_t *f)
{
	if (f->in == f->out)
		return 0;
	if (f->in < f->out)
		return (f->out - f->in);
	else
		return f->size - (f->in - f->out);
}

unsigned char fifo_put(fifo_buffer_t *f, unsigned char data)
{
	unsigned char *write = f->out;
	*write = data;
	f->out++;
	
	if (f->out > f->buffer + f->size - 1)
		f->out = f->buffer;

	if (f->out == f->in)
	{
		f->in++;
		//printf("Buffer overflow");
		if (f->in > f->buffer + f->size - 1)
			f->in = f->buffer;
		return 0;
	}

	return 1;
}

unsigned char fifo_get(fifo_buffer_t *f, unsigned char *data, unsigned long size)
{
	if (fifo_getsize(f) < size)
		return 0;

	if (((f->buffer + f->size) - f->in) < size)
	{
		unsigned long thissize = ((f->buffer + f->size) - f->in);
		memcpy(data, f->in, thissize);
		f->in = f->buffer;
		data += thissize;
		memcpy(data, f->in, size-thissize);
		f->in += size-thissize;
	}
	else
	{
		memcpy(data, f->in, size);
		f->in += size;
	}
	
	if (f->in > f->buffer + f->size - 1)
		f->in = f->buffer;
	
	return 1;
}
