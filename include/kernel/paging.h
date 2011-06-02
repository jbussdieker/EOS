////////////////////////////////////////////////////////////////////////////////
// $Id: paging.h,v 1.5 2010/12/20 10:03:38 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////
#ifndef _PAGING_H
#define _PAGING_H

void *paging_makeidentity();
void paging_map(unsigned long *, void *, void *, unsigned long, int);
void paging_debug(unsigned long *pd);

#endif
