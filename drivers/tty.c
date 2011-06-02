////////////////////////////////////////////////////////////////////////////////
// $Id: tty.c,v 1.10 2010/12/28 00:51:39 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <kernel/arch.h>
#include <kernel/fs.h>
#include <kernel/dev.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
//#define TTY_BUFFER_SIZE 256000
#define TTY_BUFFER_SIZE 64000
//#define TTY_BUFFER_SIZE 4000

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
typedef struct tty_handle_t
{
	fs_node_t fs;
	void *base;
	char buffer[TTY_BUFFER_SIZE];	// 1024000
	int buffery;
	int viewline;
	int maxline;
	int x, y;
	int sx, sy, sby;
	int width, height;
	int color;
	int attr;
	int *codepage;
	int g0, g1, g2, g3;
} tty_handle_t;

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
static unsigned int tty_write(void *p, void *buffer, size_t size);

////////////////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////////////////
tty_handle_t ttystartupdev = {
	.fs.write = tty_write, .fs.blocksize = 1, 
	.base = (void *)0xC00B80A0, .x = 0, .y = 0, .width = 80, .height = 24, .color = 0x1F,
	.viewline = 0, .maxline = 0, .buffery = 0,
	.g0 = 0, .g1 = 0, .g2 = 0, .g3 = 0,
	.codepage = &ttystartupdev.g0
};
tty_handle_t *ttystartup = &ttystartupdev;

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
static void setcursor(int x, int y)
{
	unsigned short position=(y*80) + x;
	
	// cursor LOW port to vga INDEX register
	outportb(0x3D4, 0x0F);
	outportb(0x3D5, (unsigned char)(position&0xFF));
	// cursor HIGH port to vga INDEX register
	outportb(0x3D4, 0x0E);
	outportb(0x3D5, (unsigned char )((position>>8)&0xFF));
}

void tty_scrollup()
{
	ttystartup->viewline--;
	if (ttystartup->viewline < 0)
		ttystartup->viewline = 0;
	
	memcpy(ttystartup->base, ttystartup->buffer + ttystartup->viewline * ttystartup->width * 2, ttystartup->width*ttystartup->height*2);
	setcursor(0,0);
}

void tty_scrolldown()
{
	ttystartup->viewline++;
	if (ttystartup->viewline > ttystartup->maxline)
		ttystartup->viewline = ttystartup->maxline;
		
	memcpy(ttystartup->base, ttystartup->buffer + ttystartup->viewline * ttystartup->width * 2, ttystartup->width*ttystartup->height*2);	
	setcursor(0,0);
}

void tty_clear()
{
	wmemset((void *)0xC00B8000, 0x1F00, 80*25);
	wmemset((void *)0xC00B8000, 0x4F00, 80);
	wmemset((void *)ttystartup->buffer, 0x1F00, TTY_BUFFER_SIZE / 2);
	setcursor(0,0);
	ttystartup->x = 0;
	ttystartup->y = 0;
	ttystartup->buffery = 0;
}

void tty_cls(tty_handle_t *h)
{
	wmemset((void *)h->base, h->color << 8, h->width*h->height);
	wmemset((void *)h->buffer, h->color << 8, TTY_BUFFER_SIZE / 2);
}

static unsigned char translate_color(unsigned char c)
{
	if (c==1)
		return 4;
	if (c==4)
		return 1;
	if (c==6)
		return 3;
	if (c==3)
		return 6;
}

static unsigned char translate_code(unsigned char c)
{
	if (c=='j')
		return 0xD9;
	if (c=='k')
		return 0xBF;
	if (c=='l')
		return 0xDA;
	if (c=='m')
		return 0xC0;
	if (c=='p')
		return 0xC4;
	if (c=='q')
		return 0xC4;
	if (c=='r')
		return 0xC4;
	if (c=='s')
		return 0xC4;
	if (c=='v')
		return 0xC1;
	if (c=='w')
		return 0xC2;
	if (c=='x')
		return 0xB3;
	
	//if (c==' ')
		//return 0xDB;
	return c;
}

static unsigned int tty_write(void *p, void *buffer, size_t size)
{
	tty_handle_t *h;
	h = (tty_handle_t *)p;
	unsigned char *msg = (unsigned char *)buffer;
	unsigned char *tty;
	unsigned char *ttybuffer;
	extern void *serialdev;
	
	while (*msg != 0)
	{
		tty = h->base;
		ttybuffer = h->buffer;

		// VT100 Emulation
		if (*msg == 0xF)
		{
			h->codepage = &h->g0;
			msg++;
			continue;
		}
		if (*msg == 0xE)
		{
			h->codepage = &h->g1;
			msg++;
			continue;
		}
		if (*msg == 0x1B)
		{
			msg++;
			if (*msg == 'M')
			{
				// Scroll up
				if (h->y > 0)
				{
					h->y--;
					h->buffery--;
				}
				msg++;
				continue;
			}
			if (*msg == 'D')
			{
				// Scroll up
				if (h->y < h->height - 1)
				{
					h->y++;
					h->buffery++;
				}
				msg++;
				continue;
			}
			if (*msg == '(')
			{
				msg++;
				if (*msg == 'B')
				{
					h->g0 = 0;
					fs_write(serialdev, "US char set as G0\n", 18);
				}
				if (*msg == '0')
				{
					h->g0 = 1;
					fs_write(serialdev, "line char set as G0\n", 20);
				}
				msg++;
				continue;
			}
			if (*msg == ')')
			{
				msg++;
				if (*msg == 'B')
				{
					h->g1 = 0;
					fs_write(serialdev, "US char set as G1\n", 18);
				}
				if (*msg == '0')
				{
					h->g1 = 1;
					fs_write(serialdev, "line char set as G1\n", 20);
				}
				msg++;
				continue;
			}
			if (*msg == '[')
			{
				msg++;
				
				// Make sure we didn't reach the end yet
				if (*msg == 0)
					break;
				
				// Mark parameters and skip over them
				char *parmstart = msg;
				while (isdigit(*msg) || *msg == ';')
					msg++;
				
				if (*msg == 0)
				{
					assert(0); // Error
				}
				////////////////////////////
				// Setup
				////////////////////////////
				else if (*msg == '?')
				{
					msg+=2;
				}
				////////////////////////////
				// Set Cursor
				////////////////////////////
				else if ((*msg == 'H') ||  (*msg == 'f'))
				{
					if (parmstart != msg)
					{
						unsigned char x=0, y=0;
						y = atoi(parmstart);
		
						// Skip over numeric parameter that was just read
						while (isdigit(*parmstart))
							parmstart++;
						// Second arg?
						if (*parmstart == ';')
						{
							// Skip over the colon
							parmstart++;
							x = atoi(parmstart);
							// Skip over numeric parameter that was just read
							while (isdigit(*parmstart))
								parmstart++;
						}
						// Error Check
						if ((*parmstart != 'H') && (*parmstart != 'f'))
							assert(0); //break; // Error

						h->x = x;
						h->y = h->buffery = y;
					}
					else
					{
						
					}									
				}
				////////////////////////////
				// Cursor Up
				////////////////////////////
				else if (*msg == 'J')
				{
					unsigned char parm = atoi(parmstart);
					if (parm == 2)
						tty_cls(h);
				}
				////////////////////////////
				// Cursor Up
				////////////////////////////
				else if (*msg == 'A')
				{
					unsigned char count = atoi(parmstart);
					// Clip count to the value of y
					if (h->y < count)
						count = h->y;
					h->y-=count;
					h->buffery-=count;
				}
				////////////////////////////
				// Cursor Down
				////////////////////////////
				else if (*msg == 'B')
				{
					unsigned char count = atoi(parmstart);
					// Clip count to the value of y
					if ((h->y + count) > (h->height - 1))
						count = h->height - 1 - h->y;
					h->y+=count;
					h->buffery+=count;
				}
				////////////////////////////
				// Cursor Forward
				////////////////////////////
				else if (*msg == 'C')
				{
					unsigned char count = atoi(parmstart);
					// Clip count to the value of y
					if ((h->x + count) > (h->width - 1))
						count = h->width - 1 - h->x;
					h->x+=count;
				}
				////////////////////////////
				// Cursor Backward
				////////////////////////////
				else if (*msg == 'D')
				{
					unsigned char count = atoi(parmstart);
					// Clip count to the value of y
					if (h->x < count)
						count = h->x;
					h->x-=count;
				}
				////////////////////////////
				// Save Cursor Position
				////////////////////////////
				else if (*msg == 's')
				{
					h->sx = h->x;
					h->sy = h->y;
					h->sby = h->buffery;
				}
				////////////////////////////
				// Restore Cursor Position
				////////////////////////////
				else if (*msg == 'u')
				{
					h->x = h->sx;
					h->y = h->sy;
					h->buffery = h->sby;
				}
				////////////////////////////
				// Set Graphics Mode
				////////////////////////////
				else if (*msg == 'm')
				{
					unsigned char tmp;
					while (1)
					{
						tmp = atoi(parmstart);
						if (tmp >= 0 && tmp <= 8)
						{
							if (tmp == 0)
							{
								if (h->attr == 7)
									h->color = (h->color & 0x80) | ((h->color & 7) << 4) | ((h->color >> 4) & 7);
								h->color ^= 0x8;
								h->attr = 0;
							}
							else if (tmp == 1)
								h->color |= 0x8;
							else if (tmp == 7)
							{
								h->color = (h->color & 0x80) | ((h->color & 7) << 4) | ((h->color >> 4) & 7);
								h->attr = tmp;
							}
						}
						if (tmp >= 30 && tmp <= 37)
						{
							h->color = (h->color & 0xF8) | translate_color(tmp - 30);
						}
						if (tmp >= 40 && tmp <= 47)
							h->color = ((h->color & 0xF) | (translate_color(tmp - 40) << 4));	
						
						// Skip over numeric parameter that was just read
						while (isdigit(*parmstart))
							parmstart++;
						
						// Check if we have another parameter or terminate
						if (*parmstart == 'm')
							break; // Normal
						if (*parmstart != ';')
							break; // Error
						
						// Skip over the colon
						parmstart++;
					}
				}
				msg++;
				continue;
			}
		}
		// END VT100 Emulation
		
		if (*msg == '\n')
		{
			h->y++;
			h->buffery++;
			h->x = 0;
		}
		else if (*msg == '\f')
		{
			msg++;
			continue;
		}
		else if (*msg == '\r')
		{
			h->y++;
			h->buffery++;
			h->x = 0;
		}
		else if (*msg == '\b')
		{
			if (h->x > 0)
				h->x--;
			else if (h->y > 0)
			{
				h->y--;
				h->buffery--;
				h->x = h->width - 1;
			}
			
			tty += (h->y * h->width * 2) + h->x * 2;
			*tty = ' ';
			ttybuffer += (h->buffery * h->width * 2) + h->x * 2;
			*ttybuffer = ' ';		
		}
		else
		{
			if ((h->viewline + h->height) > h->buffery)
			{
				tty += (h->y * h->width * 2) + h->x * 2;
				if (*h->codepage == 1)
					*tty++ = translate_code(*msg);
				else
					*tty++ = *msg;
				*tty = h->color;
			}

			ttybuffer += (h->buffery * h->width * 2) + h->x * 2;
			
			if (*h->codepage == 1)
				*ttybuffer++ = translate_code(*msg);
			else
				*ttybuffer++ = *msg;
			*ttybuffer = h->color;

			h->x++;
		}
		if (h->x > (h->width))
		{
			h->x = 1;
			h->y++;
			h->buffery++;
		}

		// Have we reached the last line of the buffer
		if (h->buffery >= TTY_BUFFER_SIZE / (h->width * 2))
		{
			h->buffery--;
			memcpy(h->buffer, (void *)(h->buffer + (h->width * 2)), TTY_BUFFER_SIZE - (h->width * 2));
			wmemset(h->buffer + TTY_BUFFER_SIZE - (h->width * 2), 0x1F00, h->width);
			h->maxline--;
		}

		if (h->y > (h->height-1))
		{
			h->y--;
			memcpy(h->base, (void *)(h->base + (h->width * 2)), (h->height - 1) * (h->width * 2));
			wmemset(h->base + (h->width * 2) * (h->height - 1), 0x1F00, h->width);

			h->maxline++;
			h->viewline++;
		}

		// Limit check view line
		if (h->viewline > h->maxline)
			h->viewline = h->maxline;
			
		msg++;
		setcursor(h->x, h->y+1);
	}
	
	return 0;
}

fs_node_t *tty_init(unsigned long base, int width, int height, int color)
{
	kprintf(1, "tty_init(base: 0x%X, width: %d, height: %d, color: 0x%02X)\n", base, width, height, color);
	return (fs_node_t *)ttystartup;
}
