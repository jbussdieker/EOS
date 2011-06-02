////////////////////////////////////////////////////////////////////////////////
// $Id: elf.h,v 1.4 2010/12/20 08:49:14 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////
#ifndef _ELF_H
#define _ELF_H

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
void elf_load_driver(void *p);
void elf_init_kernel(void *p);
char *elf_get_symbol(void *p);
void *elf_get_symbol_addr(char *symbol);

#endif