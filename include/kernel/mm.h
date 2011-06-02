////////////////////////////////////////////////////////////////////////////////
// $Id: mm.h,v 1.4 2010/12/18 02:06:00 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////
#ifndef _MM_H
#define _MM_H

#define PHYS_VIRT(a) ((((unsigned long)a) + 0xC0000000))
#define VIRT_PHYS(a) ((((unsigned long)a) - 0xC0000000))

void mm_init(unsigned long size);
void *mm_malloc();

#endif
