////////////////////////////////////////////////////////////////////////////////
// $Id: pci.c,v 1.16 2010/12/28 00:46:59 Ecco Exp $
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
#include <kernel/fs.h>
#include <assert.h>
#include <stdio.h>
#include "pcilist.h"

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define DEBUG

// HOST INTERFACE PORTS
#define PCI_CONFIG_ADDRESS	0xCF8	
#define PCI_CONFIG_DATA		0xCFC

// CONFIGURATION SPACE
#define PCI_VENDORID		0x00
#define PCI_DEVICEID		0x02
#define PCI_COMMAND			0x04
#define PCI_STATUS			0x06
#define PCI_REVISION		0x08
#define PCI_INTERFACE		0x09
#define PCI_SUBCLASS		0x0A
#define PCI_BASECLASS		0x0B
#define PCI_HEADERTYPE		0x0E

// COMMAND Register Flags
#define PCI_ENABLEIO		0x01
#define PCI_ENABLEMEM		0x02
#define PCI_BUSMASTER		0x04

// BAR Register Flags
#define BAR_MEM 			0x0
#define BAR_IO  			0x1
#define BAR_MEM32 			0x0
#define BAR_MEM64 			0x2
#define BAR_PREFETCHABLE 	0x8

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
typedef struct pci_bar_t
{
	unsigned long raw;
	int type;
	union
	{
		unsigned short io;
		unsigned long mem;
	} base;
	union
	{
		unsigned short io;
		unsigned long mem;
	} size;
} pci_bar_t;

typedef struct pci_device_t
{
	fs_node_t fs;
	int bus;
	int dev;
	int func;
	unsigned short vendor;
	unsigned short device;
	unsigned char base_class;
	unsigned char sub_class;
	unsigned char interface;
	pci_bar_t bars[6];
	struct pci_device_t *next;
} pci_device_t;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
static pci_device_t *first_dev = 0;

////////////////////////////////////////////////////////////////////////////////
// Static Functions
////////////////////////////////////////////////////////////////////////////////
static unsigned long pci_make_address(unsigned short bus, unsigned short dev, unsigned short func, unsigned short offset)
{
	unsigned long address;
	unsigned long lbus = (unsigned long)bus;
	unsigned long ldev = (unsigned long)dev;
	unsigned long lfunc = (unsigned long)func;
	address = (unsigned long)((lbus << 16) | (ldev << 11) | (lfunc << 8) | (offset /* VMWARE Does it... & 0xfc*/) | ((unsigned long)0x80000000));
	return address;
}

static unsigned char pci_read_byte(unsigned short bus, unsigned short dev, unsigned short func, unsigned short offset)
{
	outport(PCI_CONFIG_ADDRESS, pci_make_address(bus, dev, func, offset));
	return (unsigned char)((inport(PCI_CONFIG_DATA) >> ((offset & 3) * 8)) & 0xFF);
}

static unsigned short pci_read_word(unsigned short bus, unsigned short dev, unsigned short func, unsigned short offset)
{
	outport(PCI_CONFIG_ADDRESS, pci_make_address(bus, dev, func, offset));
	return (unsigned short)((inport(PCI_CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}

static unsigned long pci_read_dword(unsigned short bus, unsigned short dev, unsigned short func, unsigned short offset)
{
	outport(PCI_CONFIG_ADDRESS, pci_make_address(bus, dev, func, offset));
	return inport(PCI_CONFIG_DATA);
}

static void pci_write_dword(unsigned short bus, unsigned short dev, unsigned short func, unsigned short offset, unsigned long value)
{
	outport(PCI_CONFIG_ADDRESS, pci_make_address(bus, dev, func, offset));
	outport(PCI_CONFIG_DATA, value);
}

static void pci_write_word(unsigned short bus, unsigned short dev, unsigned short func, unsigned short offset, unsigned short value)
{
	outport(PCI_CONFIG_ADDRESS, pci_make_address(bus, dev, func, offset));
	outportw(PCI_CONFIG_DATA, value);
}

static void readbars(pci_device_t *dev)
{
	unsigned long tmpbar;
	unsigned long size;
	unsigned short offset;
	int i;
	
	for (i = 0; i < 6; i++)
	{
		// Calculate BAR offset (Each BAR is 4 bytes and BAR0 starts at 0x10)
		offset = 0x10 + i * 4;

		// Read the initial value of the BAR (Skip entries that are 0)
		dev->bars[i].raw = pci_read_dword(dev->bus,dev->dev,dev->func,offset);
		if (dev->bars[i].raw == 0)
			continue;
			
		// Write all 1's to the register then read it back (Not sure if we'll ever get zero here)
		pci_write_dword(dev->bus,dev->dev,dev->func,offset,0xFFFFFFFF);
		tmpbar = pci_read_dword(dev->bus,dev->dev,dev->func,offset);
		assert(tmpbar != 0);

		if ((tmpbar & 1) == BAR_IO)
		{
			// Mask off flags
			tmpbar = tmpbar & 0xFFFFFFFC;
			
			// Fill io fields
			dev->bars[i].size.io = (tmpbar ^ 0xFFFFFFFF) + 1;
			dev->bars[i].base.io = (dev->bars[i].raw & 0xFFFC);
			dev->bars[i].type = BAR_IO;
		}
		else if ((tmpbar & 1) == BAR_MEM)
		{
			// Determine type and whether it's prefetchable
			int type = (tmpbar >> 1) & 0x03;
			//int prefetchable = (tmpbar >> 3) & 1;
			
			// Currently the only supported type of BAR memory is 32-bit
			if (type == BAR_MEM32)
			{
				tmpbar = tmpbar & 0xFFFFFFF0;
				dev->bars[i].size.mem = (tmpbar ^ 0xFFFFFFFF) + 1;
				dev->bars[i].base.mem = (dev->bars[i].raw & 0xFFFFFFF0);
				dev->bars[i].type = BAR_MEM;
			}
			else if (type == BAR_MEM64)
				printf("readbars: Unexpected MEM64 PCI device\n");
			else
				printf("readbars: Unexpected PCI BAR type\n");
		}
		
		// Restore the value of the BAR register
		pci_write_dword(dev->bus,dev->dev,dev->func,offset,dev->bars[i].raw);
	}
}

////////////////////////////////////////////////////////////////////////////////
// Debug Functions
////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
void pcilist()
{
	pci_device_t *cur = first_dev;
	int i = 0;
	printf("PCI Device List\n");
	while (cur != NULL)
	{
		printf(" %02d - ", i);
		i++;
		printf("(%d, %d, %d) \x1B[0m", cur->bus, cur->dev, cur->func);
		if (cur->base_class < KNOWN_BASECLASS_COUNT)
		{
			if (cur->base_class != 0)
			{
				if (sub_class[cur->base_class-1][cur->sub_class] != 0)
					printf("%s ", sub_class[cur->base_class-1][cur->sub_class]);
			}
			printf("%s ", class_codes[cur->base_class]);
		}
		printf("\n\x1B[1m");
		cur = cur->next;
	}
}

void pci(int index)
{
	pci_device_t *cur = first_dev;
	int i = 0;
	while (cur != NULL)
	{
		if (i == index)
		{
			printf("PCI Device (%d, %d, %d)\n", cur->bus, cur->dev, cur->func);

			printf(" Vendor: \x1B[0m");
			if (get_vendor_string(cur->vendor) == NULL)
				printf("%04X\n", cur->vendor);
			else
				printf("%s\n", get_vendor_string(cur->vendor));

			printf(" \x1B[1mDevice: \x1B[0m");
			if (get_device_string(cur->vendor, cur->device) == NULL)
				printf("%04X\n", cur->device);
			else
				printf("%s\n", get_device_string(cur->vendor, cur->device));

			printf (" \x1B[1mClass: \x1B[0m");
			if (cur->base_class < KNOWN_BASECLASS_COUNT)
			{
				if (cur->base_class != 0)
				{
					if (sub_class[cur->base_class-1][cur->sub_class] != 0)
						printf("%s ", sub_class[cur->base_class-1][cur->sub_class]);
				}
				printf("%s\n", class_codes[cur->base_class]);
			}
			else
				printf("Base: %02X Sub: %02X\n", cur->base_class, cur->sub_class);
			
			printf(" \x1B[1mInterface:\x1B[0m %02X\n", cur->interface);
			printf(" \x1B[1mBase Address Registers\n");
			int i;
			for (i = 0; i < 6; i++)
			{
				if (cur->bars[i].raw != 0)
				{
					printf("  \x1B[1mBAR%d Type:\x1B[0m ", i);
					if (cur->bars[i].type == BAR_IO)
						printf("IO  \x1B[1mBase:\x1B[0m 0x%X \x1B[1mSize:\x1B[0m 0x%X", cur->bars[i].base.io, cur->bars[i].size.io);
					else if (cur->bars[i].type == BAR_MEM)
						printf("MEM \x1B[1mBase:\x1B[0m 0x%08X \x1B[1mSize:\x1B[0m %X", cur->bars[i].base.mem, cur->bars[i].size.mem);
					printf("\n\x1B[1m");
				}
			}
			return;
		}
		cur = cur->next;
		i++;
	}
}
#endif // DEBUG

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
// Allocate device structures for all PCI devices
void pci_init()
{
	kprintf(1, "pci_init()\n", 0);
	
	int bus, dev, func;

	for (bus = 0; bus < 8; bus++)
	for (dev = 0; dev < 32; dev++)
	for (func = 0; func < 8; func++)
	{
		unsigned short vendor;
		
		if ((vendor = pci_read_word(bus, dev, func, PCI_VENDORID)) != 0xFFFF)
		{					
			// Create new handle
			pci_device_t *pcidev;
			char dev_name[32];
			memset(dev_name, 0, 32);
			sprintf(dev_name, "pci%d,%d,%d", bus, dev, func);
			pcidev = (pci_device_t *)dev_alloc(dev_name, sizeof(pci_device_t));
		
			// Fill universal fields
			pcidev->fs.blocksize = 1;
		
			// Fill address
			pcidev->bus = bus;
			pcidev->dev = dev;
			pcidev->func = func;
			
			// Fill custom fields
			pcidev->vendor = vendor;
			pcidev->device = pci_read_word(bus, dev, func, PCI_DEVICEID);
			pcidev->base_class = pci_read_byte(bus, dev, func, PCI_BASECLASS);
			pcidev->sub_class = pci_read_byte(bus, dev, func, PCI_SUBCLASS);
			pcidev->interface = pci_read_byte(bus, dev, func, PCI_INTERFACE);
			
			// Read the base address register information
			readbars(pcidev);
			
			// Add device to pci list			
			if (first_dev == NULL)
				// Set root device
				first_dev = pcidev;
			else
			{
				// Root exists add to end
				pci_device_t *cur = first_dev;
				while (cur->next != NULL)
					cur = cur->next;
				cur->next = pcidev;
			}
		}
	}
}

// Enable IO Space, Memory Space, and Bus Master
// \parm p Pointer to the PCI device
void pci_enable_mem(pci_device_t *dev)
{
	unsigned short cmd_reg;
	
	// Read command register
	cmd_reg = pci_read_word(dev->bus, dev->dev, dev->func, PCI_COMMAND);
	
	// Enable flags
	cmd_reg |= PCI_ENABLEIO;
	cmd_reg |= PCI_ENABLEMEM;
	cmd_reg |= PCI_BUSMASTER;
	
	// Write command register back
	pci_write_word(dev->bus, dev->dev, dev->func, PCI_COMMAND, cmd_reg);
}

// Return the give base address register (address part)
// \parm p Pointer to the PCI device
// \parm index Index of the desired BAR
unsigned long pci_get_bar_address(pci_device_t *dev, int index)
{
	unsigned long save;
	unsigned short offset = 0x10 + index * 4;
	
	// Read the BAR register
	save = pci_read_dword(dev->bus,dev->dev,dev->func,offset);
	assert(save != 0);

	// Return the IO or Memory address
	if (save & 1)
		return (save & 0xFFFFFFFC);
	else
		return (save & 0xFFFFFFF0);
}

// Finds a matching vendor and device for device driver lookups
// \parm vendor Desired device vendor ID
// \parm device Desired device ID
fs_node_t *pci_get_device(pci_device_t *cur, unsigned short vendor, unsigned short device)
{
	if (cur == 0)
		cur = first_dev;
	else
		cur = cur->next;

	while (cur != NULL)
	{
		if ((cur->vendor == vendor) && 
			(cur->device == device))
			return cur;
		cur = cur->next;
	}
	
	return NULL;
}

// Finds a matching device class
// \parm base_class Desired base class
// \parm sub_class Desired sub class
// \parm interface Desired interface
pci_device_t *pci_get_type(pci_device_t *cur, unsigned char base_class, unsigned char sub_class, unsigned char interface)
{
	if (cur == 0)
		cur = first_dev;
	else
		cur = cur->next;
		
	while (cur != NULL)
	{
		if ((cur->base_class == base_class) && 
			(cur->sub_class == sub_class) &&
			(cur->interface == interface))
			return cur;
		cur = cur->next;
	}
	
	return NULL;
}
