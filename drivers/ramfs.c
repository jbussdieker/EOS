////////////////////////////////////////////////////////////////////////////////
// $Id: ramfs.c,v 1.3 2010/12/17 06:00:14 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <kernel/dev.h>
#include <kernel/fs.h>

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
fs_node_t *fsroot = 0;

////////////////////////////////////////////////////////////////////////////////
// Static Functions
////////////////////////////////////////////////////////////////////////////////
static fs_node_t *ramfs_alloc(fs_node_t *parent, char *name, int flags)
{
	// Create the node and zero it
	fs_node_t *node;
	node = malloc(sizeof(fs_node_t));
	memset(node, 0, sizeof(fs_node_t));
	
	// Fill in attributes
	strcpy(node->name, name);
	node->flags |= flags;
	
	// Linked list work
	node->parent = parent;
	
	if (parent->first_child == 0)
	{
		node->prev = 0;
		node->next = 0;
		parent->first_child = node;
	}
	else
	{
		fs_node_t *addbefore = parent->first_child;
		node->prev = 0;
		node->next = addbefore;
		addbefore->prev = node;
		parent->first_child = node;
	}
	
	return node;
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
fs_node_t *ramfs_open(fs_node_t *dev, const char *filename)
{
	if (filename != 0)
	{
		fs_node_t *curdev = dev;
		
		curdev = curdev->next;
		while (curdev != fsroot)
		{
			if (strcmp(curdev->name, filename) == 0)
				return curdev;

			curdev = curdev->next;
		}

		return 0;
	}
	
	fs_node_t *rdev;
	rdev = malloc(sizeof(fs_node_t));
	memcpy(rdev, dev, sizeof(fs_node_t));
	rdev->ptr = rdev->first_child;

	return rdev;
}

struct dirent *ramfs_readdir(void *d)
{
	fs_node_t *dev = (fs_node_t *)d;
	if (dev->ptr == NULL)
		return 0;

	strcpy(dev->dir.name, dev->ptr->name);
	dev->ptr = dev->ptr->next;
	return &dev->dir;
}

void ramfs_dump(int level, fs_node_t *root)
{
	fs_node_t *dev = root->first_child;
	while (dev != 0)
	{
		printf(" %*s%s\n", level, "", dev->name);
		if (dev->flags & FS_FLAG_DIRECTORY)
			ramfs_dump(level + 1, dev);
		dev = dev->next;
	}
}

fs_node_t *ramfs_init()
{
	// Create new handle
	fs_node_t *dev;
	dev = malloc(sizeof(fs_node_t));
	memset(dev, 0, sizeof(fs_node_t));
	
	//dev = (ramfs_node_t *)dev_alloc("ramfs", sizeof(ramfs_node_t));

	// Fill universal fields
	dev->open = ramfs_open;
	dev->readdir = ramfs_readdir;
	dev->blocksize = 1;
	dev->flags |= FS_FLAG_DIRECTORY;
	dev->next = dev;
	dev->prev = dev;
	devroot = ramfs_alloc(dev, "dev", FS_FLAG_DIRECTORY);

	return dev;
}
