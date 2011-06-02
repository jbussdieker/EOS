////////////////////////////////////////////////////////////////////////////////
// $Id: pic.c,v 1.1 2010/12/23 09:22:01 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/arch.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
// Registers
#define PIC_MASTER_CMD	0x20
#define PIC_MASTER_DATA	0x21
#define PIC_SLAVE_CMD	0xA0
#define PIC_SLAVE_DATA	0xA1

// Commands
#define PIC_CMD_INIT	0x10
#define PIC_CMD_EOI		0x20

// Flags
#define PIC_ICW4		0x01
#define PIC_8086		0x01
#define PIC_S2			0x04

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void pic_init()
{
	// Send ICW1
	// Set initialize and use 4 bytes init flags
	outportb(PIC_MASTER_CMD, PIC_CMD_INIT | PIC_ICW4); 
	outportb(PIC_SLAVE_CMD, PIC_CMD_INIT | PIC_ICW4);

	// Send ICW2
	// Remap the interrupts for protected mode
	// IRQ 0-7 remapped to IVT 32-39
	outportb(PIC_MASTER_DATA, 32);
	// IRQ 8-15 remapped to IVT 40-47
	outportb(PIC_SLAVE_DATA, 40);

	// Send ICW3
	// Set S2 bit to indicate int 2 cascades
	outportb(PIC_MASTER_DATA, PIC_S2);
	// Set value 2 to indicate int 2 in master
	outportb(PIC_SLAVE_DATA, 2);

	// Send ICW4
	// Just set the 8086 type flags
	outportb(PIC_MASTER_DATA, PIC_8086);
	outportb(PIC_SLAVE_DATA, PIC_8086);

	// Unmask all interrupts
	outportb(PIC_MASTER_DATA, 0x00);
	outportb(PIC_SLAVE_DATA, 0x00);
}

void pic_ack(int irq)
{
	// Send non-specific EOI
	outportb(PIC_MASTER_CMD, PIC_CMD_EOI);
	if (irq >= 8)
		outportb(PIC_SLAVE_CMD, PIC_CMD_EOI);
}
