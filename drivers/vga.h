////////////////////////////////////////////////////////////////////////////////
// $Id: vga.h,v 1.11 2010/12/22 10:51:54 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////
#ifndef _VGA_H
#define _VGA_H

typedef struct vga_handle_t
{
	fs_node_t fs;
	int (*set_mode)(struct vga_handle_t *, int, int, int);
	void (*unset_mode)(struct vga_handle_t *);
	void (*flip)(struct vga_handle_t *);
	void (*clear)(struct vga_handle_t *, unsigned long);
	void (*pset)(struct vga_handle_t *, int, int, unsigned long);
	void (*filledrect)(struct vga_handle_t *, int, int, int, int, unsigned long);
	int width;
	int height;
	int bpp;
	void *savebuffer;
	void *offset;
	void *bb;
} vga_handle_t;

void vga_pset(void *h, int x, int y, unsigned long pixel);
void vga_filledrect(void *h, int x, int y, int width, int height, unsigned long pixel);
void vga_rect(void *h, int x, int y, int width, int height, unsigned long pixel);
void vga_clear(void *h, unsigned long color);
void vga_set_mode(void *, int, int, int);
void *vga_get_base(void *h);
void *vga_init();
void vga_flip(void *h);
void vga_draw_string(void *buffer, int x, int y, int color, char *str, ...);
void vga_unset_mode(void *h);

#endif