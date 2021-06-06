

#include <stdio.h>
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"

extern APTR video_mutex;

extern void _free_fake_bitmap( struct BitMap *bm );

void SetWinLayer( struct Window *win );

void fake_SizeWindow ( struct Window *win, LONG dx, LONG dy )
{
	MutexObtain(video_mutex);
	no_block_SizeWindow ( win, dx, dy );
	MutexRelease(video_mutex);
}

void no_block_SizeWindow ( struct Window *win, LONG dx, LONG dy )
{
	int w,h;

	w = win -> Width + dx;
	h = win -> Height + dy;

	if (w> win -> MaxWidth) w = win -> MaxWidth;
	if (h> win -> MaxHeight) h = win -> MaxHeight;

	if (w< win -> MinWidth) w = win -> MinWidth;
	if (h< win -> MinHeight) h = win -> MinHeight;

	dx = w - win -> Width;
	dy = h - win -> Height;

	if (dx | dy)
	{
		SetAPen(win -> RPort,0);
		ClearBorder(win);
		win -> Width = w;
		win -> Height = h;
		win -> GZZWidth = win -> Width - win -> BorderLeft - win -> BorderRight;
		win -> GZZHeight = win -> Height - win -> BorderTop - win -> BorderBottom;
		SetWinLayer( win );
		RenderWindow(win);
	}
}

void SetWinLayer( struct Window *win )
{
	struct Layer *l;

	l = win -> RPort -> Layer;
	if (l)
	{
		LONG x,y,w,h;
		LONG max_x,max_y;
		LONG dw,dh;
		struct Rectangle *r;

		r = &(l -> bounds);

		x = r -> MinX;
		y = r -> MinY;
		w = win -> Width + 1;
		h = win -> Height + 1;

		max_x = x + w ;
		max_y = y + h ;

		if (max_x > win -> WScreen -> Width )
		{
			max_x = win -> WScreen -> Width;
			w = max_x - x;
		}

		if (max_y > win -> WScreen -> Height )
		{
			max_y = win -> WScreen -> Height;
			h = max_y - y;
		}

		dw = w - l -> Width;
		dh = h - l -> Height;

		MoveSizeLayer(  l, 0, 0,dw,dh );
	}
}

