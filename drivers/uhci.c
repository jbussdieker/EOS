////////////////////////////////////////////////////////////////////////////////
// $Id: uhci.c,v 1.4 2010/12/28 02:23:07 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <kernel/mm.h>
#include <kernel/dev.h>
#include <stdlib.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define DEBUG

#define USBCMD			0x00
#define USBSTS			0x02
#define USBINTR			0x04
#define FRNUM			0x06
#define FLBASEADD		0x08
#define SOFMOD			0x0C
#define PORTSC0			0x10
#define PORTSC1			0x12

// USBCMD
#define CMD_RS			0x1
#define CMD_HCRESET		0x2
#define CMD_GRESET		0x4
#define CMD_EGSM		0x8
#define CMD_FGR			0x10
//#define CMD_SWDBG		0x20
//#define CMD_SWDBG		0x40
#define CMD_MAXP		0x80

// USBSTS
#define STS_HCHALTED	0x20
#define STS_INT			0x1

#define SUSPEND			0x10
#define PORT_RESET		0x2
#define LOW_SPEED		0x1

#define RESUME_DETECT	0x40
#define PORT_CHANGE		0x8
#define PORT_ENABLED	0x4
#define CONNECT_CHANGE	0x2
#define CONNECT_STATUS	0x1

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
typedef struct uhci_device_t
{
	fs_node_t fs;
	void *pci;
	unsigned short iobase;
	void *framelist;
	struct uhci_device_t *next;
} uhci_device_t;

typedef struct usb_req_t
{
	unsigned char bmRequestType;
	unsigned char bRequest;
	unsigned short wValue;
	unsigned short wIndex;
	unsigned short wLength;
} usb_req_t;

typedef struct usb_td_t
{
	unsigned long t:1;
	unsigned long q:1;
	unsigned long vf:1;
	unsigned long res_0:1;
	unsigned long linkpointer:28;
		
	unsigned long feedback;
	
	unsigned long pid:8;
	unsigned long devaddr:7;
	unsigned long endpt:4;
	unsigned long d:1;
	unsigned long r:1;
	unsigned long maxlen:11;
	
	unsigned long bufferptr;
} usb_td_t;

typedef struct usb_qh_t
{
	unsigned long head_t:1;
	unsigned long head_q:1;
	unsigned long head_res_0:2;
	unsigned long head_linkpointer:28;

	unsigned long elem_t:1;
	unsigned long elem_q:1;
	unsigned long elem_res_0:2;
	unsigned long elem_linkpointer:28;
} usb_qh_t;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
uhci_device_t *first_dev;

////////////////////////////////////////////////////////////////////////////////
// Debug Functions
////////////////////////////////////////////////////////////////////////////////
#ifdef DEBUG
static uhci_device_t *uhci_get_index(int index)
{
	uhci_device_t *cur = first_dev;
	int i = 0;
	while (cur != NULL)
	{
		if (i == index)
			return cur;
		i++;
		cur = cur->next;
	}
	return NULL;
}

void uhcilist()
{
	uhci_device_t *cur = first_dev;
	int i = 0;
	while (cur != NULL)
	{
		printf(" %02d - \x1B[0m", i);
		i++;
		printf("%s", cur->fs.name);
		printf("\x1B[1m\n");
		cur = cur->next;
	}
}

void uhci(int index)
{
	uhci_device_t *cur = uhci_get_index(index);
	if (cur == NULL)
		return;
	
	printf("UHCI Host Controller\n");
	printf(" USBCMD:    \x1B[0m%04X\x1B[1m\n", inportw(cur->iobase+USBCMD));
	printf(" USBSTS:    \x1B[0m%04X ", inportw(cur->iobase+USBSTS));
	if (inportw(cur->iobase+USBSTS) & STS_HCHALTED)
		printf("\n  STS_HCHALTED");
	printf("\x1B[1m\n");
	
	printf(" USBINTR:   \x1B[0m%04X\x1B[1m\n", inportw(cur->iobase+USBINTR));
	printf(" FRNUM:     \x1B[0m%04X\x1B[1m\n", inportw(cur->iobase+FRNUM));
	printf(" FLBASEADD: \x1B[0m%08X\x1B[1m\n", inport(cur->iobase+FLBASEADD));
	printf(" SOFMOD:    \x1B[0m%02X\x1B[1m\n", (unsigned char)inportb(cur->iobase+SOFMOD));
	
	unsigned short portsc0, portsc1;
	portsc0 = inportw(cur->iobase+PORTSC0);
	portsc1 = inportw(cur->iobase+PORTSC1);
	
	printf(" PORTSC0:   \x1B[0m");
	if (portsc0 & SUSPEND)
		printf("SUSPEND ");
	if (portsc0 & PORT_RESET)
		printf("PORT_RESET ");
	if (portsc0 & LOW_SPEED)
		printf("LOW_SPEED ");
	if (portsc0 & RESUME_DETECT)
		printf("RESUME_DETECT ");
	if (portsc0 & CONNECT_STATUS)
		printf("CONNECT_STATUS ");
	if (portsc0 & CONNECT_CHANGE)
		printf("CONNECT_CHANGE ");
	if (portsc0 & PORT_ENABLED)
		printf("PORT_ENABLED ");
	if (portsc0 & PORT_CHANGE)
		printf("PORT_CHANGE ");
	printf("\x1B[1m\n");

	printf(" PORTSC1:   \x1B[0m");
	if (portsc1 & SUSPEND)
		printf("SUSPEND ");
	if (portsc1 & PORT_RESET)
		printf("PORT_RESET ");
	if (portsc1 & LOW_SPEED)
		printf("LOW_SPEED ");
	if (portsc1 & RESUME_DETECT)
		printf("RESUME_DETECT ");
	if (portsc1 & CONNECT_STATUS)
		printf("CONNECT_STATUS ");
	if (portsc1 & CONNECT_CHANGE)
		printf("CONNECT_CHANGE ");
	if (portsc1 & PORT_ENABLED)
		printf("PORT_ENABLED ");
	if (portsc1 & PORT_CHANGE)
		printf("PORT_CHANGE ");
	printf("\x1B[1m\n");

	printf(" PORTSC0:   \x1B[0m%04X\x1B[1m\n", portsc0);
	printf(" PORTSC1:   \x1B[0m%04X\x1B[1m\n", portsc1);
}

void uhcirun(int index)
{
	uhci_device_t *cur = uhci_get_index(index);
	if (cur == NULL)
		return;

	unsigned short tmp = inportw(cur->iobase+USBCMD);
	tmp |= CMD_RS;
	outportw(cur->iobase+USBCMD, tmp);
}

void uhcistop(int index)
{
	uhci_device_t *cur = uhci_get_index(index);
	if (cur == NULL)
		return;

	unsigned short tmp = inportw(cur->iobase+USBCMD);
	tmp &= (~CMD_RS);
	outportw(cur->iobase+USBCMD, tmp);
}

void uhcireset(int index)
{
	uhci_device_t *dev = uhci_get_index(index);
	if (dev == NULL)
		return;

	outportw(dev->iobase+USBCMD, CMD_HCRESET);
}

void uhcitest(int index)
{
	uhci_device_t *dev = uhci_get_index(index);
	if (dev == NULL)
		return;

	// Test code
	unsigned long *x = dev->framelist;
	usb_qh_t *qh = malloc(sizeof(usb_qh_t));
	usb_td_t *td = malloc(sizeof(usb_td_t));
	void *test = malloc(4096);
	
	// Pointer is invalid 
	qh->head_t = 1;
	qh->head_q = 0; // Don't care
	qh->head_linkpointer = 0; // Don't care
	
	// Pointer is valid
	qh->elem_t = 0;
	qh->elem_q = 0;
	qh->elem_linkpointer = VIRT_PHYS(td) >> 4;

	// Pointer is invalid
	td->t = 1;
	td->q = 0; // Don't care
	td->vf = 1; // Don't care
	td->linkpointer = 0;  // Don't care
	
	td->pid = 0xFF; //0x2D;
	td->maxlen = 4;
	td->r = 0;
	td->d = 0;
	td->endpt = 0;
	td->devaddr = 0;
	td->bufferptr = VIRT_PHYS(test);
	
	*x = VIRT_PHYS(td);
}

void uhcidebugtd(unsigned long *td)
{
	printf("  0: %08X\n", *td);
	printf("  1: %08X\n", *(td+1));
	printf("  2: %08X\n", *(td+2));
	printf("  3: %08X\n", *(td+3));
}

void uhcidebugqh(unsigned long *qh)
{
	printf(" Elem: %08X\n", *qh);
	if ((*qh & 1) == 0)
	{
		if (*qh & 2)
			uhcidebugqh(PHYS_VIRT(*qh & 0xFFFFFFF0));
		else
			uhcidebugtd(PHYS_VIRT(*qh & 0xFFFFFFF0));
	}
	printf(" Link: %08X\n", *(qh+1));
	if ((*(qh+1) & 1) == 0)
	{
		if (*(qh+1) & 2)
			uhcidebugqh(PHYS_VIRT(*(qh+1) & 0xFFFFFFF0));
		else
			uhcidebugtd(PHYS_VIRT(*(qh+1) & 0xFFFFFFF0));
	}
}

void flist(int index)
{
	uhci_device_t *cur = uhci_get_index(index);
	if (cur == NULL)
		return;

	unsigned long *tmp = cur->framelist;
	int j;
	for (j = 0; j < 10; j++)
	{
		printf("Frame %d: %08X\n", j, *tmp);
		
		// Valid Frame Pointer
		if ((*tmp & 1) == 0)
		{
			// QH
			if (*tmp & 2)
				uhcidebugqh(PHYS_VIRT(*tmp & 0xFFFFFFF0));
			// TD
			else
				uhcidebugtd(PHYS_VIRT(*tmp & 0xFFFFFFF0));
		}
		
		tmp++;
	}
}
#endif // DEBUG

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void uhci_init()
{
	void *pci;
	pci = pci_get_type(NULL, 0xC, 0x3, 0x00);
	pci_enable_mem(pci);
	while (pci != 0)
	{
		kprintf(2, "found USB (UHCI) host controller\n", 0);

		// Create device structure
		uhci_device_t *dev;
		dev = (uhci_device_t *)dev_alloc("uhci", sizeof(uhci_device_t));
		dev->pci = pci;
		dev->iobase = pci_get_bar_address(pci, 4);
		dev->framelist = PHYS_VIRT(mm_malloc());

		// Reset
		outportw(dev->iobase+USBCMD, CMD_HCRESET);

		// Clear the frame list
		memsetd(dev->framelist, 1, 1024);
		

		// Set framelist pointer
		outport(dev->iobase+FLBASEADD, VIRT_PHYS(dev->framelist));
		
		
		// Add device to uhci list			
		if (first_dev == NULL)
			// Set root device
			first_dev = dev;
		else
		{
			// Root exists add to end
			uhci_device_t *cur = first_dev;
			while (cur->next != NULL)
				cur = cur->next;
			cur->next = dev;
		}
		
		// Get next UHCI controller
		pci = pci_get_type(pci, 0xC, 0x3, 0x00);
	}
	
	uhcitest(0);
	//uhcirun(0);
}
