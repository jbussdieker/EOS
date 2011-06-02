////////////////////////////////////////////////////////////////////////////////
// $Id: gobject.c,v 1.1 2010/12/23 09:22:03 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Structures
////////////////////////////////////////////////////////////////////////////////
typedef struct gui_object_t
{
	unsigned long ID;
	unsigned long ClassID;
	struct gui_object_t *Parent;
	struct gui_object_t *FirstChild;
	struct gui_object_t *Next;
	struct gui_object_t *Prev;
	unsigned long x, y;
	unsigned long width, height;
	char *caption;
	union
	{
		struct
		{
			// callbacks
		} window;
		struct
		{
			// callbacks
		} button;
	} sub;
} gui_object_t;

gui_object_t gui_root_node = {
	.ID = 0,
	.ClassID = 0,
	.Parent = 0,
	.FirstChild = 0,
	.Next = 0,
	.Prev = 0,
	.x = 0,
	.y = 0,
	.width = 0,
	.height = 0,
	.caption = 0
};

gui_object_t *groot = &gui_root_node;

////////////////////////////////////////////////////////////////////////////////
// Local Functions
////////////////////////////////////////////////////////////////////////////////
void add_child(gui_object_t *parent, gui_object_t *child)
{
	if (parent->FirstChild == 0)
	{
		parent->FirstChild = child;
		return;
	}
	else
	{
		gui_object_t *walk = parent->FirstChild;
		while (walk->Next != 0)
			walk = walk->Next;
		walk->Next = child;
		child->Prev = walk;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
gui_object_t *gui_create(unsigned long ClassID, gui_object_t *parent)
{
	gui_object_t *g;
	g = malloc(sizeof(gui_object_t));
	memset(g, 0, sizeof(g));
	if (parent == 0)
		g->Parent = groot;
	else
		g->Parent = parent;
	add_child(parent, g);
	g->ClassID = ClassID;
	return g;
}

void gui_recursive_render(gui_object_t *cur)
{
	// Render
	if (cur->Next != 0)
		gui_recursive_render(cur->Next);
	if (cur->FirstChild != 0)
		gui_recursive_render(cur->FirstChild);
}

void gui_render()
{
	gui_recursive_render(groot);
}
