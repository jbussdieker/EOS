////////////////////////////////////////////////////////////////////////////////
// $Id: syscall.h,v 1.3 2010/12/18 02:06:00 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////
#ifndef _SYSCALL_H
#define _SYSCALL_H

#define SYS_OPEN		1
#define SYS_READ		2
#define SYS_WRITE		3
#define SYS_SEEK		4
#define SYS_CLOSE		5
#define SYS_READDIR		7

#define SYS_SLEEP		9

#define SYS_CD			10

unsigned long syscall(unsigned long, unsigned long, unsigned long, unsigned long);
unsigned long syscall_process(unsigned long callid, unsigned long parm1, unsigned long parm2, unsigned long parm3);

#endif
