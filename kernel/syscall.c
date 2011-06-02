////////////////////////////////////////////////////////////////////////////////
// $Id: syscall.c,v 1.1 2010/12/18 01:18:01 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description:
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/syscall.h>
#include <kernel/tasks.h>

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
unsigned long syscall_process(unsigned long callid, unsigned long parm1, unsigned long parm2, unsigned long parm3)
{
	switch (callid)
	{
		case SYS_OPEN:
			return (unsigned long)fs_open((void *)curtask->node, (void *)parm1);
			break;
		case SYS_READ:
			return fs_read((void *)parm1, (void *)parm2, parm3);
			break;
		case SYS_WRITE:
			return fs_write((void *)parm1, (void *)parm2, parm3);
			break;
		case SYS_SEEK:
			fs_seek((void *)parm1, parm2);
			break;
		case SYS_CLOSE:
			fs_close((void *)parm1);
			break;
		case SYS_READDIR:
			return (unsigned long)fs_readdir((void *)parm1);
			break;
		case SYS_SLEEP:
			task_scheduler();
			break;
		case SYS_CD:
			curtask->node = fs_open(curtask->node, (void *)parm1);
			break;
		default:
			break;
	}
	
	return 0;
}
