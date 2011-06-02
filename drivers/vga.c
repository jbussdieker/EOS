////////////////////////////////////////////////////////////////////////////////
// $Id: vga.c,v 1.16 2010/12/22 10:51:54 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <kernel/kernel.h>
#include <kernel/paging.h>
#include <kernel/tasks.h>
#include <kernel/fs.h>
#include <kernel/mm.h>
#include "vga.h"
#include "vga-font.h"

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
static void vga_draw_char(void *buffer, int sx, int sy, unsigned char c, int color)
{
	unsigned char *fontent = &g_8x8_font[c*8];
	int x, y;
	int offset;
	for (y = 0; y < 8; y++)
	{
		for (x = 0; x < 8; x++)
		{
			// y * 8 + x
			offset = (y << 3) + x;
			// (fontent[offset/8] & (1 << (offset%8))
			if (fontent[offset >> 3] & (0x80 >> (offset&7)))
				vga_pset(buffer, sx+x, sy+y, color);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void vga_pset(void *h, int x, int y, unsigned long pixel)
{
	vga_handle_t *dev = (vga_handle_t *)h;

	// Clipping Code
	if (x>=dev->width)
		return;
	if (y>=dev->height)
		return;
	if (x<0)
		return;
	if (y<0)
		return;
	assert(x>=0);assert(y>=0);
	assert(x<dev->width);assert(y<dev->height);

	if (dev->pset != 0)
		dev->pset(dev, x, y, pixel);
	else
	{
		// Drawing Code
		unsigned long *ptr = dev->bb;
		ptr += y * dev->width + x;
		*ptr = pixel;
	}
}

void vga_filledrect(void *h, int x, int y, int width, int height, unsigned long pixel)
{
	vga_handle_t *dev = (vga_handle_t *)h;

	// Clipping Code
	if (x >= dev->width)
		return;
	if (y >= dev->height)
		return;
	if (x + width < 0)
		return;
	if (x+width>dev->width)
		width = dev->width - x;
	if (y+height>dev->height)
		height = dev->height - y;
	if (x < 0)
		width += x;
	//assert(x>=0); //assert(y>=0);
	assert(x<dev->width);assert(y<dev->height);
	assert(x+width<=dev->width);assert(y+height<=dev->height);
	
	if (dev->filledrect != 0)
		dev->filledrect(h, x, y, width, height, pixel);
	else
	{	
		// Drawing Code
		unsigned long *ptr = dev->bb;
		ptr += y * dev->width;
		if (x > 0)
			ptr += x;
		int i;
		for (i = 0; i < height; i++)
		{
			if ((y + i) < 0)
			{
				ptr += dev->width;
				continue;
			}
	
			memsetd(ptr, pixel, width);
			ptr += dev->width;
		}
	}
}

void vga_rect(void *h, int x, int y, int width, int height, unsigned long pixel)
{
	vga_handle_t *dev = (vga_handle_t *)h;

	// Clipping Code
	if (x>=dev->width)
		return;
	if (y>=dev->height)
		return;
	if (x+width>dev->width)
		width = dev->width - x;
	if (y+height>dev->height)
		height = dev->height - y;
	if (x+width<=0)
		return;
	if (x < 0)
		width += x;
	//assert(x>=0); //assert(y>=0);
	assert(x<dev->width);assert(y<dev->height);
	assert(x+width<=dev->width);assert(y+height<=dev->height);
	
	// Drawing Code
	unsigned long *ptr = dev->bb;
	ptr += y * dev->width;
	if (x > 0)
		ptr += x;
	int i;
	for (i = 0; i < height; i++)
	{
		if ((y + i) < 0)
		{
			ptr += dev->width;
			continue;
		}
			
		if ((i == 0) || (i == (height - 1)))
		{
			memsetd(ptr, pixel, width);
		}
		else
		{
			if (x > 0)
				*ptr = pixel;
			if (x + width <= dev->width)
				*(ptr + width - 1) = pixel;
		}
		ptr += dev->width;
	}
}

void vga_clear(void *h, unsigned long color)
{
	vga_handle_t *dev = (vga_handle_t *)h;
	
	if (dev->clear != 0)
		dev->clear(dev, color);
	else
	{
		if (dev->bpp == 32)
			memsetd(dev->bb, color, dev->width * dev->height);
		else
			memset(dev->bb, color, dev->width * dev->height * (dev->bpp / 8));
	}
}

void vga_unset_mode(void *h)
{
	vga_handle_t *dev = (vga_handle_t *)h;
	if (dev->unset_mode != 0)
		dev->unset_mode(dev);
	free(dev->bb);
	dev->bb = 0;
}

void vga_set_mode(void *h, int width, int height, int bpp)
{
	vga_handle_t *dev = (vga_handle_t *)h;
	if (dev->set_mode != 0)
	{
		if (dev->set_mode(dev, width, height, bpp))
		{
			dev->width = width;
			dev->height = height;
			dev->bpp = bpp;

			if (dev->bpp > 8)
			{
				// Bochs / VMWare
				dev->bb = malloc(dev->width * dev->height * (dev->bpp / 8));
				paging_map((void *)PHYS_VIRT(curtask->cr3), dev->offset, dev->offset, 1920*1200*4, 7);
			}
			else if (dev->bpp == 4)
			{
				// Generic
				dev->bb = malloc(dev->width * dev->height / 2);
			}
		}
		else
		{
			kprintf(0, "vga_set_mode() failed\n", 0);
		}
	}
}

void vga_flip(void *h)
{
	vga_handle_t *dev = (vga_handle_t *)h;
	
	if (dev->flip != 0)
		dev->flip(dev);
	else
		memcpy(dev->offset, dev->bb, (dev->width * dev->height)*4);
}

void vga_draw_string(void *buffer, int x, int y, int color, char *str, ...)
{
	char finalbuffer[128];
	va_list args;
	va_start(args, str);
	vsprintf(finalbuffer, str, args);
	va_end(args);
	char *s = finalbuffer;

	int cursor_x = x;
	int cursor_y = y;
	while (*s != '\0')
	{
		if (*s == '\n')
		{
			cursor_y += 8;
			cursor_x = x;
		}
		else
		{
			vga_draw_char(buffer, cursor_x, cursor_y, *s, color);
			cursor_x += 8;
		}
		
		s++;
	}
}


void *vga_init()
{
	kprintf(1, "vga_init()\n", 0);

	void *dev;
	fs_node_t *vga_bochs_init();
	fs_node_t *vga_vmware_init();
	fs_node_t *vga_generic_init();

	/*dev = vga_bochs_init();
	if (dev != 0)
		return dev;
	dev = vga_vmware_init();
	if (dev != 0)
		return dev;	*/
	
	return vga_generic_init();

	//kprintf(0, "vga_init() failed to find suitable VGA card\n", 0);
	
	//return 0;
}
