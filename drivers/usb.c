////////////////////////////////////////////////////////////////////////////////
// $Id: usb.c,v 1.2 2010/12/23 10:31:38 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void usb_init()
{
	kprintf(1, "usb_init()\n", 0);
	uhci_init();
	ohci_init();
	ehci_init();
}
