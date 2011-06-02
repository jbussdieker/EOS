////////////////////////////////////////////////////////////////////////////////
// $Id: assert.h,v 1.2 2010/12/17 01:16:44 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////
#ifndef _ASSERT_H
#define _ASSERT_H

#include <stdio.h>

#define assert(expr) if ((expr)==0) printf("Assertion failed: " #expr ", file %s, line %d\n", __FILE__, __LINE__);

#endif