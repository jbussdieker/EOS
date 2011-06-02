////////////////////////////////////////////////////////////////////////////////
// $Id: vga-vmware.c,v 1.12 2010/12/23 09:22:02 Ecco Exp $
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
#include <assert.h>
#include "vga.h"
#include "pci.h"

#define PACKED 
#include "vga-vmware.h"

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
typedef struct vgavmware_handle_t
{
	vga_handle_t vga;
	fs_node_t *pci;
	unsigned long version;
	unsigned long iobase;
	unsigned long fb;
	unsigned long fifo;
	unsigned long fbsize;
	unsigned long fifosize;
	unsigned long pitch;
} vgavmware_handle_t;

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
void SVGA_WriteReg(vgavmware_handle_t *dev, uint32 index, uint32 value)
{
   outport(dev->iobase + SVGA_INDEX_PORT, index);
   outport(dev->iobase + SVGA_VALUE_PORT, value);
}

uint32
SVGA_ReadReg(vgavmware_handle_t *dev, uint32 index)  // IN
{
   outport(dev->iobase + SVGA_INDEX_PORT, index);
   return inport(dev->iobase + SVGA_VALUE_PORT);
}

void
SVGA_SetMode(vgavmware_handle_t *dev, uint32 width,   // IN
             uint32 height,  // IN
             uint32 bpp)     // IN
{
   SVGA_WriteReg(dev, SVGA_REG_WIDTH, width);
   SVGA_WriteReg(dev, SVGA_REG_HEIGHT, height);
   SVGA_WriteReg(dev, SVGA_REG_BITS_PER_PIXEL, bpp);
   SVGA_WriteReg(dev, SVGA_REG_ENABLE, 1);
   dev->pitch = SVGA_ReadReg(dev, SVGA_REG_BYTES_PER_LINE);
}

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
int vga_vmware_set_mode(vga_handle_t *dev, int width, int height, int bpp)
{
	SVGA_SetMode((vgavmware_handle_t *)dev, width, height, bpp);
	return 1;
}

void vga_vmware_unset_mode(vga_handle_t *dev)
{
   SVGA_WriteReg((vgavmware_handle_t *)dev, SVGA_REG_ENABLE, 0);
}

fs_node_t *vga_vmware_init(int width, int height, int bpp)
{
	// Find the device and get it's BAR addresses
	fs_node_t *pci = 0;
	pci = pci_get_device(PCI_VENDOR_ID_VMWARE, PCI_DEVICE_ID_VMWARE_SVGA2);
	if (pci == 0)
		return 0;
	
	// Create new handle
	vgavmware_handle_t *dev;
	dev = (vgavmware_handle_t *)dev_alloc("vga", sizeof(vgavmware_handle_t));

	// Fill universal fields
	dev->vga.set_mode = vga_vmware_set_mode;
	dev->vga.unset_mode = vga_vmware_unset_mode;
	dev->vga.fs.blocksize = 1;

	// Fill custom fields
	dev->vga.width = width;
	dev->vga.height = height;
	dev->vga.bpp = bpp;
	dev->pci = pci;
	
	//pci_enable_mem(dev->pci);
	dev->iobase = pci_get_bar_address(dev->pci, 0);
	dev->fb = pci_get_bar_address(dev->pci, 1);
	dev->vga.offset = (void *)dev->fb;
	dev->fifo = pci_get_bar_address(dev->pci, 2);
	
	// Negotiate device version
	dev->version = SVGA_ID_2;
	do
	{
		SVGA_WriteReg(dev, SVGA_REG_ID, dev->version);
		if (SVGA_ReadReg(dev, SVGA_REG_ID) == dev->version)
			break;
		else
			dev->version--;
	} while (dev->version >= SVGA_ID_0);
	
	assert(dev->version >= SVGA_ID_0);

	// Get FIFO size and FB size
	dev->fbsize = SVGA_ReadReg(dev, SVGA_REG_FB_SIZE);
	dev->fifosize = SVGA_ReadReg(dev, SVGA_REG_MEM_SIZE);
	assert(dev->fbsize >= 0x100000);
	//assert(dev->fifosize < 0x20000);

	kprintf(2, "found VMWare VGA device\n", 0);
	return (fs_node_t *)dev;
}
