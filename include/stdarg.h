////////////////////////////////////////////////////////////////////////////////
// $Id: stdarg.h,v 1.1 2010/12/12 23:57:31 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////
#ifndef _STDARG_H
#define _STDARG_H

////////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////////
#define __dj_va_rounded_size(T) (((sizeof (T) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#define va_arg(ap, T) (ap = (va_list) ((char *) (ap) + __dj_va_rounded_size (T)),	*((T *) (void *) ((char *) (ap) - __dj_va_rounded_size (T))))
#define va_end(ap)
#define va_start(ap, last_arg) (ap = ((va_list) __builtin_next_arg (last_arg)))
typedef void *va_list;


#endif
