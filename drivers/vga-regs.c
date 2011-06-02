////////////////////////////////////////////////////////////////////////////////
// $Id: vga-regs.c,v 1.2 2010/12/21 05:09:02 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/fs.h>
#include "vga.h"

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void vga_save_regs(vga_handle_t *vga)
{
/*	unsigned char misc;
	unsigned char *buf = malloc(128);
	vga->regs = buf;

	misc = inportb(VGA_MISC_READ);
	*buf = misc;
	buf++;
	
	printf("\nCRT: ");
	if (misc & VGA_IOAS)
	{
		int i;
		for (i = 0x00; i <= 0x18; i++)
		{
			outportb(VGA_CRT_INDEX, i);
			*buf = inportb(VGA_CRT_DATA);
			printf("%02X ", *buf);
			buf++;
		}
	}
	printf("\n");

	printf("GRAPHICS: ");
	int i;
	for (i = 0x00; i <= 0x08; i++)
	{
		outportb(VGA_GR_INDEX, i);
		*buf = inportb(VGA_GR_DATA);
		printf("%02X ", *buf);
		buf++;
	}
	printf("\n");

	printf("SEQUENCER: ");
	for (i = 0x00; i <= 0x04; i++)
	{
		outportb(VGA_SEQ_INDEX, i);
		*buf = inportb(VGA_SEQ_DATA);
		printf("%02X ", *buf);
		buf++;
	}
	printf("\n");*/
}

void vga_load_regs(vga_handle_t *vga)
{
/*	unsigned char misc = inportb(VGA_MISC_READ);
	unsigned char *buf = vga->regs;

	outportb(VGA_MISC_WRITE, *buf);
	buf++;

	printf("CRT: ");
	if (misc & VGA_IOAS)
	{
		int i;
		for (i = 0x00; i <= 0x18; i++)
		{
			outportb(VGA_CRT_INDEX, i);
			outportb(VGA_CRT_DATA, *buf);
			printf("%02X ", *buf);
			buf++;
		}
	}	
	printf("\n");

	printf("GRAPHICS: ");
	int i;
	for (i = 0x00; i <= 0x08; i++)
	{
		outportb(VGA_GR_INDEX, i);
		outportb(VGA_GR_DATA, *buf);
		printf("%02X ", *buf);
		buf++;
	}
	printf("\n");

	printf("SEQUENCER: ");
	for (i = 0x00; i <= 0x04; i++)
	{
		outportb(VGA_SEQ_INDEX, i);
		outportb(VGA_SEQ_DATA, *buf);
		printf("%02X ", *buf);
		buf++;
	}
	printf("\n");*/
}
