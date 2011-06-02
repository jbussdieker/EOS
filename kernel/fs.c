////////////////////////////////////////////////////////////////////////////////
// $Id: fs.c,v 1.1 2010/12/18 01:18:01 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/fs.h>

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
fs_node_t *fs_open(fs_node_t *node, const char *filename)
{
	if (node == 0)
		return 0;	
	if (node->open != 0)
		return node->open(node, filename);
	else
		return 0;
}

unsigned int fs_read(fs_node_t *node, void *buf, size_t size)
{
	if (node == 0)
		return 0;	
	if (node->read != 0)
		return node->read(node, buf, size);
	return 0;
}

struct dirent *fs_readdir(fs_node_t *node)
{
	if (node == 0)
		return 0;
	if (node->readdir != 0)
		return node->readdir(node);
	return 0;
}

unsigned int fs_write(fs_node_t *node, void *buf, size_t size)
{
	if (node == 0)
		return 0;
	if (node->write != 0)	
		return node->write(node, buf, size);
	return 0;
}

int fs_seek(fs_node_t *node, int offset)
{
	if (node == 0)
		return 0;
	if (node->seek != 0)
		return node->seek(node, offset);
	return 0;
}

int fs_close(fs_node_t *node)
{
	if (node == 0)
		return 0;
	if (node->close != 0)
		return node->close(node);
	return 0;
}
