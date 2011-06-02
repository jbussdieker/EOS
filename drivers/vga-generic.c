////////////////////////////////////////////////////////////////////////////////
// $Id: vga-generic.c,v 1.6 2010/12/23 09:22:02 Ecco Exp $
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
#include <kernel/dev.h>
#include "vga.h"

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
#define	VGA_AC_INDEX		0x3C0
#define	VGA_AC_WRITE		0x3C0
#define	VGA_AC_READ			0x3C1
#define	VGA_MISC_WRITE		0x3C2
#define VGA_SEQ_INDEX		0x3C4
#define VGA_SEQ_DATA		0x3C5
#define	VGA_DAC_READ_INDEX	0x3C7
#define	VGA_DAC_WRITE_INDEX	0x3C8
#define	VGA_DAC_DATA		0x3C9
#define	VGA_MISC_READ		0x3CC
#define VGA_GC_INDEX 		0x3CE
#define VGA_GC_DATA 		0x3CF
/*			COLOR emulation		MONO emulation */
#define VGA_CRTC_INDEX		0x3D4		/* 0x3B4 */
#define VGA_CRTC_DATA		0x3D5		/* 0x3B5 */
#define	VGA_INSTAT_READ		0x3DA

#define	VGA_NUM_SEQ_REGS	5
#define	VGA_NUM_CRTC_REGS	25
#define	VGA_NUM_GC_REGS		9
#define	VGA_NUM_AC_REGS		21
#define	VGA_NUM_REGS		(1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + \
				VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
typedef struct vgageneric_handle_t
{
	vga_handle_t vga;
	void *regs;
	void *mem0;
	void *mem1;
	void *mem2;
	void *mem3;
} vgageneric_handle_t;

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
unsigned char g_640x480x16[] =
{
/* MISC */
	0xE3,
/* SEQ */
	0x03, 0x01, 0x08, 0x00, 0x06,
/* CRTC */
	0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E,
	0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xEA, 0x0C, 0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3,
	0xFF,
/* GC */
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F,
	0xFF,
/* AC */
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
	0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
	0x01, 0x00, 0x0F, 0x00, 0x00
};

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
static void set_plane(unsigned p)
{
	unsigned char pmask;

	p &= 3;
	pmask = 1 << p;
/* set read plane */
	outportb(VGA_GC_INDEX, 4);
	outportb(VGA_GC_DATA, p);
/* set write plane */
	outportb(VGA_SEQ_INDEX, 2);
	outportb(VGA_SEQ_DATA, pmask);
}

static void read_regs(unsigned char *regs)
{
	unsigned i;

/* read MISCELLANEOUS reg */
	*regs = inportb(VGA_MISC_READ);
	regs++;
/* read SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		outportb(VGA_SEQ_INDEX, i);
		*regs = inportb(VGA_SEQ_DATA);
		regs++;
	}
/* read CRTC regs */
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		outportb(VGA_CRTC_INDEX, i);
		*regs = inportb(VGA_CRTC_DATA);
		regs++;
	}
/* read GRAPHICS CONTROLLER regs */
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		outportb(VGA_GC_INDEX, i);
		*regs = inportb(VGA_GC_DATA);
		regs++;
	}
/* read ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		(void)inportb(VGA_INSTAT_READ);
		outportb(VGA_AC_INDEX, i);
		*regs = inportb(VGA_AC_READ);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)inportb(VGA_INSTAT_READ);
	outportb(VGA_AC_INDEX, 0x20);
}

void write_regs(unsigned char *regs)
{
	unsigned i;

/* write MISCELLANEOUS reg */
	outportb(VGA_MISC_WRITE, *regs);
	regs++;
/* write SEQUENCER regs */
	for(i = 0; i < VGA_NUM_SEQ_REGS; i++)
	{
		outportb(VGA_SEQ_INDEX, i);
		outportb(VGA_SEQ_DATA, *regs);
		regs++;
	}
/* unlock CRTC registers */
	outportb(VGA_CRTC_INDEX, 0x03);
	outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) | 0x80);
	outportb(VGA_CRTC_INDEX, 0x11);
	outportb(VGA_CRTC_DATA, inportb(VGA_CRTC_DATA) & ~0x80);
/* make sure they remain unlocked */
	regs[0x03] |= 0x80;
	regs[0x11] &= ~0x80;
/* write CRTC regs */
	for(i = 0; i < VGA_NUM_CRTC_REGS; i++)
	{
		outportb(VGA_CRTC_INDEX, i);
		outportb(VGA_CRTC_DATA, *regs);
		regs++;
	}
/* write GRAPHICS CONTROLLER regs */
	for(i = 0; i < VGA_NUM_GC_REGS; i++)
	{
		outportb(VGA_GC_INDEX, i);
		outportb(VGA_GC_DATA, *regs);
		regs++;
	}
/* write ATTRIBUTE CONTROLLER regs */
	for(i = 0; i < VGA_NUM_AC_REGS; i++)
	{
		(void)inportb(VGA_INSTAT_READ);
		outportb(VGA_AC_INDEX, i);
		outportb(VGA_AC_WRITE, *regs);
		regs++;
	}
/* lock 16-color palette and unblank display */
	(void)inportb(VGA_INSTAT_READ);
	outportb(VGA_AC_INDEX, 0x20);
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
static int vga_generic_set_mode(vgageneric_handle_t *dev, int width, int height, int bpp)
{
	dev->regs = malloc(128);
	read_regs(dev->regs);

	write_regs(g_640x480x16);

	dev->mem0 = malloc(128*1024);
	dev->mem1 = malloc(128*1024);
	dev->mem2 = malloc(128*1024);
	dev->mem3 = malloc(128*1024);

	set_plane(0);
	memcpy(dev->mem0, 0xC00A0000, 128*1024);
	set_plane(1);
	memcpy(dev->mem1, 0xC00A0000, 128*1024);
	set_plane(2);
	memcpy(dev->mem2, 0xC00A0000, 128*1024);
	set_plane(3);	
	memcpy(dev->mem3, 0xC00A0000, 128*1024);

	return 1;
}

static void vga_generic_unset_mode(vgageneric_handle_t *dev)
{
	set_plane(0);
	memcpy(0xC00A0000, dev->mem0, 128*1024);
	set_plane(1);
	memcpy(0xC00A0000, dev->mem1, 128*1024);
	set_plane(2);
	memcpy(0xC00A0000, dev->mem2, 128*1024);
	set_plane(3);	
	memcpy(0xC00A0000, dev->mem3, 128*1024);
	set_plane(0);

	free(dev->mem0);
	free(dev->mem1);
	free(dev->mem2);
	free(dev->mem3);

	write_regs(dev->regs);
	free(dev->regs);
	dev->regs = 0;
}

static void vga_generic_clear(vgageneric_handle_t *dev)
{
	unsigned long bufsize;
	bufsize = dev->vga.width * dev->vga.height / 8;

	memset(dev->vga.bb, 			0x00, bufsize);
	memset(dev->vga.bb+bufsize, 	0x00, bufsize);
	memset(dev->vga.bb+bufsize*2, 	0x00, bufsize);
	memset(dev->vga.bb+bufsize*3, 	0xFF, bufsize);
}

static void vga_generic_pset(vgageneric_handle_t *dev, int x, int y, unsigned long color)
{
	char *pixel = dev->vga.bb;
	pixel += y * dev->vga.width / 8;
	pixel += x / 8;
	
	unsigned char value = (0x80 >> (x % 8));
	if (color & 8)
		*pixel |= (0x80 >> (x % 8));
	else
		*pixel &= ~(0x80 >> (x % 8));
	pixel += dev->vga.width * dev->vga.height / 8;	
	if (color & 4)
		*pixel |= (0x80 >> (x % 8));
	else
		*pixel &= ~(0x80 >> (x % 8));
	pixel += dev->vga.width * dev->vga.height / 8;	
	if (color & 2)
		*pixel |= (0x80 >> (x % 8));
	else
		*pixel &= ~(0x80 >> (x % 8));
	pixel += dev->vga.width * dev->vga.height / 8;	
	if (color & 1)
		*pixel |= (0x80 >> (x % 8));
	else
		*pixel &= ~(0x80 >> (x % 8));
}

static void vga_generic_filledrect(vgageneric_handle_t *dev, int x, int y, int width, int height, unsigned long color)
{
	if (x < 0)
		x = 0;
	
	char *ptr = dev->vga.bb;
	ptr += y * dev->vga.width / 8;
	ptr += (x+7)/8;

	int i, j, endx, total;
	for (i = 0; i < height; i++)
	{
		total = 0;
		if ((y + i) < 0)
		{
			ptr += dev->vga.width / 8;
			continue;
		}
		
		if (width == 1)
		{
			vga_generic_pset(dev, x, y+i, color);
			continue;
		}
		
		if (x % 8)
			for (j = x; j < (x/8+1)*8; j++)
			{
				vga_generic_pset(dev, j, i+y, color);
				total++;
			}
		
		// Not sure
		int setsize = (width - total) / 8;
		
		total += setsize * 8;
		if (color & 8)
			memset(&ptr[0], 0xFF, setsize);
		else
			memset(&ptr[0], 0x00, setsize);
		if (color & 4)
			memset(&ptr[dev->vga.width * dev->vga.height / 8], 0xFF, setsize);
		else
			memset(&ptr[dev->vga.width * dev->vga.height / 8], 0x00, setsize);
		if (color & 2)
			memset(&ptr[dev->vga.width * dev->vga.height / 8*2], 0xFF, setsize);
		else
			memset(&ptr[dev->vga.width * dev->vga.height / 8*2], 0x00, setsize);
		if (color & 1)
			memset(&ptr[dev->vga.width * dev->vga.height / 8*3], 0xFF, setsize);
		else
			memset(&ptr[dev->vga.width * dev->vga.height / 8*3], 0x00, setsize);

		endx = x + width;
		for (j = endx-(endx%8); j < endx; j++)
		{
			vga_generic_pset(dev, j, i+y, color);
			total++;
		}

		ptr += dev->vga.width / 8;
	}
}

static void vga_generic_flip(vgageneric_handle_t *dev)
{
	unsigned long bufsize;
	bufsize = dev->vga.width * dev->vga.height / 8;
	
	set_plane(0);
	memcpy(dev->vga.offset, dev->vga.bb, bufsize);
	set_plane(1);
	memcpy(dev->vga.offset, dev->vga.bb+bufsize, bufsize);
	set_plane(2);
	memcpy(dev->vga.offset, dev->vga.bb+bufsize*2, bufsize);
	set_plane(3);	
	memcpy(dev->vga.offset, dev->vga.bb+bufsize*3, bufsize);
}

fs_node_t *vga_generic_init()
{
	// Create new handle
	vgageneric_handle_t *dev;
	dev = (vgageneric_handle_t *)dev_alloc("vga", sizeof(vgageneric_handle_t));

	// Fill universal fields
	dev->vga.fs.blocksize = 1;
	dev->vga.set_mode = vga_generic_set_mode;
	dev->vga.unset_mode = vga_generic_unset_mode;
	dev->vga.flip = vga_generic_flip;
	dev->vga.clear = vga_generic_clear;
	dev->vga.pset = vga_generic_pset;
	dev->vga.filledrect = vga_generic_filledrect;
	dev->vga.offset = 0xC00A0000;

	kprintf(2, "found generic VGA card\n", 0);
	return (fs_node_t *)dev;
}
