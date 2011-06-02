////////////////////////////////////////////////////////////////////////////////
// $Id: gui.c,v 1.13 2010/12/22 10:51:54 Ecco Exp $
//
// Author: Joshua Bussdieker
// E-Mail: jbussdieker@gmail.com
// Description: 
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include <kernel/kernel.h>
#include <kernel/syscall.h>
#include <kernel/tasks.h>
#include <kernel/paging.h>
#include <kernel/mm.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "../drivers/vga.h"
#include "../drivers/mouse.h"

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
#define COLOR1 0x00424142
#define COLOR2 0x00848284
#define COLOR3 0x00D6D3CE
#define COLOR4 0x00FFFFFF
#define SELECT 0x0008246B
#define COLORBG 0x00396DA5

#define GUI_WIDTH 640
#define GUI_HEIGHT 480
#define GUI_BPP 4

////////////////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////////////////
char cursor[21][12] = {
	{1,0,0,0,0,0,0,0,0,0,0,0},
	{1,1,0,0,0,0,0,0,0,0,0,0},
	{1,2,1,0,0,0,0,0,0,0,0,0},
	{1,2,2,1,0,0,0,0,0,0,0,0},
	{1,2,2,2,1,0,0,0,0,0,0,0},
	{1,2,2,2,2,1,0,0,0,0,0,0},
	{1,2,2,2,2,2,1,0,0,0,0,0},
	{1,2,2,2,2,2,2,1,0,0,0,0},
	{1,2,2,2,2,2,2,2,1,0,0,0},
	{1,2,2,2,2,2,2,2,2,1,0,0},
	{1,2,2,2,2,2,2,2,2,2,1,0},
	{1,2,2,2,2,2,2,1,1,1,1,1},
	{1,2,2,2,1,2,2,1,0,0,0,0},
	{1,2,2,1,1,2,2,1,0,0,0,0},
	{1,2,1,0,0,1,2,2,1,0,0,0},
	{1,1,0,0,0,1,2,2,1,0,0,0},
	{1,0,0,0,0,0,1,2,2,1,0,0},
	{0,0,0,0,0,0,1,2,2,1,0,0},
	{0,0,0,0,0,0,0,1,2,2,1,0},
	{0,0,0,0,0,0,0,1,2,2,1,0},
	{0,0,0,0,0,0,0,0,1,1,0,0}
	};

/*char folder[16][16] = {
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
	};*/

char folder[16][16] = {
	{0,0,1,1,1,1,1,0,0,0,0,0,0,0,0,0},
	{0,1,3,3,3,3,3,1,0,0,0,0,0,0,0,0},
	{0,1,3,3,3,3,3,1,1,1,1,1,1,0,0,0},
	{1,3,3,3,3,3,3,3,3,3,3,3,3,1,0,0},
	{1,3,3,3,1,1,1,1,1,1,1,1,1,1,1,0},
	{1,3,3,1,3,3,3,3,3,3,3,3,3,3,3,1},
	{1,3,3,1,3,3,3,3,3,3,3,3,3,3,3,1},
	{1,3,3,1,3,3,3,3,3,3,3,3,3,3,3,1},
	{1,3,1,3,3,3,3,3,3,3,3,3,3,3,1,0},
	{1,3,1,3,3,3,3,3,3,3,3,3,3,3,1,0},
	{1,3,1,3,3,3,3,3,3,3,3,3,3,3,1,0},
	{1,1,3,3,3,3,3,3,3,3,3,3,3,1,0,0},
	{1,1,3,3,3,3,3,3,3,3,3,3,3,1,0,0},
	{1,1,3,3,3,3,3,3,3,3,3,3,3,1,0,0},
	{1,3,3,3,3,3,3,3,3,3,3,3,1,0,0,0},
	{1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0}
	};
extern fs_node_t *vgadev;

////////////////////////////////////////////////////////////////////////////////
// Functions
////////////////////////////////////////////////////////////////////////////////
static void gui_draw_window(void *b, int x, int y, int width, int height)
{
	vga_filledrect(b, x, y, width - 1, 1, COLOR3);
	vga_filledrect(b, x + 1, y + 1, width - 3, 1, COLOR4);
	vga_filledrect(b, x, y + 1, 1, height - 2, COLOR3);
	vga_filledrect(b, x + 1, y + 2, 1, height - 4, COLOR4);
	vga_filledrect(b, x + width - 2, y + 1, 1, height - 3, COLOR2);
	vga_filledrect(b, x + width - 1, y, 1, height - 1, COLOR1);
	vga_filledrect(b, x + 1, y + height - 2, width - 2, 1, COLOR2);
	vga_filledrect(b, x, y + height - 1, width, 1, COLOR1);
	vga_filledrect(b, x+2, y+2, width-4, height-4, COLOR3);
	vga_filledrect(b, x+3, y+3, width-6, 18, SELECT);
	vga_draw_string(b, x+7, y+7, COLOR4, "Ticks %d", kinfo.ticks);
	vga_draw_string(b, x+7, y+27, COLOR4, "X %d Y %d", x+2, y+2);
}

static void gui_draw_widget(void *b, int x, int y)
{
	int i,j;
	for (i = 0; i < 21; i++)
	{
		for (j = 0; j < 12; j++)
		{
			if (cursor[i][j] == 1)
				vga_pset(b, x+j, y+i, 0x00000000);
			if (cursor[i][j] == 2)
				vga_pset(b, x+j, y+i, 0xFFFFFFFF);
		}
	}
}

static void gui_draw_icon(void *b, int x, int y)
{
	int i,j;
	for (i = 0; i < 16; i++)
	{
		for (j = 0; j < 16; j++)
		{
			if (folder[i][j] != 0)
			{
				if (folder[i][j] == 1)
					vga_pset(b, x+j, y+i, 0);
				else
					vga_pset(b, x+j, y+i, folder[i][j]);
			}
		}
	}
}


int x, lx;
int y, ly;
int winx = 100;
int winy = 100;
int winlx = 100;
int winly = 100;
int running = 0;

void gui_daemon(int argc, char **argv)
{
	mouse_packet_t packet;
	extern fs_node_t *mousedev;

	while(1)
	{
		// HACK for real computer to skip this
		if (mousedev == 0)
			continue;
			
		syscall(SYS_READ, (unsigned long)mousedev, (unsigned long)&packet, sizeof(packet));
		lx = x;
		ly = y;
		x += (packet.x);
		y -= (packet.y);
		if (x < 0)
			x=0;
		if (y < 0)
			y=0;
		if (x > GUI_WIDTH-1)
			x=GUI_WIDTH-1;
		if (y > GUI_HEIGHT-1)
			y=GUI_HEIGHT-1;	

		if ((packet.buttons) & 1)
		{
			winlx = winx;
			winly = winy;
			winx += x-lx;
			winy -= ly-y;
		}
	}
}

void gui_main(int argc, char **argv)
{
	extern unsigned long BootPageDirectory;
	paging_map(&BootPageDirectory, (void *)0xC0400000, (void *)0x400000, 1024*1024*8, 0x7);


	lx = x = GUI_WIDTH/2;
	ly = y = GUI_HEIGHT/2;
	x = y = 50;

	task_create(gui_daemon, 0, "moused", 0);
	
	unsigned long frames = 0;
	unsigned long tickstart = kinfo.ticks;
	while (1)
	{
		if (running == 0)
		{
			if (kinfo.gui == 1)
			{
				vga_set_mode(vgadev, GUI_WIDTH, GUI_HEIGHT, GUI_BPP);
				running = 1;
				tickstart = kinfo.ticks;
				frames = 0;
			}
			continue;
		}
		else if (kinfo.gui == 0)
		{
			vga_unset_mode(vgadev);
			running = 0;
			tty_scrolldown();
			continue;
		}
		
		frames++;
		
		// Clear
		vga_clear(vgadev, COLORBG);
		
		// Destop text
		vga_draw_string(vgadev, 0, 0, COLOR4, "EOS (Built %s %s)\n", __DATE__, __TIME__);
		vga_draw_string(vgadev, 0, 8, COLOR4, "Processor: %s", kinfo.cpuname);
		vga_draw_string(vgadev, 0, 16, COLOR4, "Memory: %dMB", kinfo.memory / 1024);
		vga_draw_string(vgadev, 0, 24, COLOR4, "Memory Used: %d", mm_used());

		vga_draw_string(vgadev, 0, 40, COLOR4, "mSPF %d", (kinfo.ticks-tickstart)/frames);
		if (kinfo.ticks - tickstart > 100)
			vga_draw_string(vgadev, 0, 48, COLOR4, "FPS %d", frames / ((kinfo.ticks-tickstart)/100));
		
		// Desktop icons
		//gui_draw_icon(vgadev, 8, 8);
		int winxnow, winynow;
		winxnow = winx;
		winynow = winy;
		gui_draw_window(vgadev, winxnow, winynow, 320, 240);
		gui_draw_icon(vgadev, winxnow+24, winynow+64);
		vga_draw_string(vgadev, winxnow+64, winynow+68, COLOR4, "Devices");
		gui_draw_icon(vgadev, winxnow+24, winynow+96);
		vga_draw_string(vgadev, winxnow+64, winynow+100, COLOR4, "Users");

		gui_draw_widget(vgadev, x, y);
		vga_flip(vgadev);
	}
}
