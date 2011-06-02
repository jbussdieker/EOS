////////////////////////////////////////////////////////////////////////////////
// $Id: vga-bochs.c,v 1.13 2010/12/23 09:22:02 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <kernel/arch.h>
#include <kernel/dev.h>
#include <stdlib.h>
#include <string.h>
#include "vga.h"

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define VBE_DISPI_INDEX_ID (0)
#define VBE_DISPI_INDEX_XRES (1)
#define VBE_DISPI_INDEX_YRES (2)
#define VBE_DISPI_INDEX_BPP (3)
#define VBE_DISPI_INDEX_ENABLE (4)
#define VBE_DISPI_INDEX_BANK (5)
#define VBE_DISPI_INDEX_VIRT_WIDTH (6)
#define VBE_DISPI_INDEX_VIRT_HEIGHT (7)
#define VBE_DISPI_INDEX_X_OFFSET (8)
#define VBE_DISPI_INDEX_Y_OFFSET (9) 

#define VBE_DISPI_BPP_4 (0x04)
#define VBE_DISPI_BPP_8 (0x08)
#define VBE_DISPI_BPP_15 (0x0F)
#define VBE_DISPI_BPP_16 (0x10)
#define VBE_DISPI_BPP_24 (0x18)
#define VBE_DISPI_BPP_32 (0x20) 

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

#define VBE_DISPI_DISABLED (0x00)
#define VBE_DISPI_ENABLED (0x01)
#define VBE_DISPI_ID4 (0xB0C4)
#define VBE_DISPI_ID5 (0xB0C5)
#define VBE_DISPI_LFB_ENABLED (0x41)
#define VBE_DISPI_NOCLEARMEM (0x80)

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
static void BgaWriteRegister(unsigned short IndexValue, unsigned short DataValue)
{
    outportw(VBE_DISPI_IOPORT_INDEX, IndexValue);
    outportw(VBE_DISPI_IOPORT_DATA, DataValue);
}
 
static unsigned short BgaReadRegister(unsigned short IndexValue)
{
    outportw(VBE_DISPI_IOPORT_INDEX, IndexValue);
    return inportw(VBE_DISPI_IOPORT_DATA);
}
 
static int BgaIsAvailable(void)
{
	unsigned short ver = BgaReadRegister(VBE_DISPI_INDEX_ID);
	if (ver == VBE_DISPI_ID4)
		return 1;
	if (ver == VBE_DISPI_ID5)
		return 1;
	return 0;
}
 
static void BgaSetVideoMode(unsigned int Width, unsigned int Height, unsigned int BitDepth, int UseLinearFrameBuffer, int ClearVideoMemory)
{
    BgaWriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    BgaWriteRegister(VBE_DISPI_INDEX_XRES, Width);
    BgaWriteRegister(VBE_DISPI_INDEX_YRES, Height);
    BgaWriteRegister(VBE_DISPI_INDEX_BPP, BitDepth);
    BgaWriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED |
        (UseLinearFrameBuffer ? VBE_DISPI_LFB_ENABLED : 0) |
        (ClearVideoMemory ? 0 : VBE_DISPI_NOCLEARMEM));
}
/*
static void BgaSetBank(unsigned short BankNumber)
{
    BgaWriteRegister(VBE_DISPI_INDEX_BANK, BankNumber);
}
*/
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
static int vga_bochs_set_mode(vga_handle_t *dev, int width, int height, int bpp)
{
	// Actually change the mode
	dev->savebuffer = malloc(80*25*2);
	memcpy(dev->savebuffer, (void *)0xC00B8000, 80*25*2);
	BgaSetVideoMode(width, height, bpp, 1, 1);
	return 1;
}

static void vga_bochs_unset_mode(vga_handle_t *dev)
{
	// Actually change the mode
    BgaWriteRegister(VBE_DISPI_INDEX_ENABLE, 0);
	memcpy((void *)0xC00B8000, dev->savebuffer, 80*25*2);
	free(dev->savebuffer);
}

fs_node_t *vga_bochs_init()
{
	if (BgaIsAvailable() == 0)
		return 0;
	
	// Create new handle
	vga_handle_t *dev;
	dev = (vga_handle_t *)dev_alloc("vga", sizeof(vga_handle_t));

	// Fill universal fields
	dev->fs.blocksize = 1;
	dev->set_mode = vga_bochs_set_mode;
	dev->unset_mode = vga_bochs_unset_mode;

	// Fill custom fields
	dev->offset = (void *)0xE0000000;

	kprintf(2, "found bochs VBE device\n", 0);
	return (fs_node_t *)dev;
}
