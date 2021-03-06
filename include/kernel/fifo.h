////////////////////////////////////////////////////////////////////////////////
// $Id: fifo.h,v 1.1 2010/12/20 10:05:17 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////
#ifndef _FIFO_H
#define _FIFO_H

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
typedef struct fifo_buffer_t
{
	int size;
	void *buffer;
	void *in;
	void *out;
} fifo_buffer_t;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
fifo_buffer_t *		fifo_alloc(int size);
unsigned char		fifo_put(fifo_buffer_t *f, unsigned char data);
unsigned char 		fifo_get(fifo_buffer_t *f, unsigned char *data, unsigned long size);
unsigned long 		fifo_getsize(fifo_buffer_t *f);

#endif
