

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

void fake_MoveWindow ( struct Window *win, LONG dx, LONG dy )
{
	MutexObtain(video_mutex);
	no_block_MoveWindow ( win, dx, dy );
	MutexRelease(video_mutex);
}

void no_block_MoveWindow ( struct Window *win, LONG dx, LONG dy )
{
	struct Layer *l;
	LONG max_x,max_y;
	LONG dw,dh;

	l = win -> RPort -> Layer;
	if (l)
	{
		LONG x,y,w,h;
		struct Rectangle *r;
		r = &(l -> bounds);

		w = win -> Width + 1;
		h = win -> Height + 1;

		x = r -> MinX + dx;
		y = r -> MinY + dy;

		if (x<0)
		{
			x= 0;
			dx = - r -> MinX;
		}

		if (y<0)
		{
			y= 0;
			dy = -r -> MinY;
		}

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

		MoveSizeLayer(  l, dx, dy,dw,dh );

		win -> LeftEdge =  r -> MinX;
		win -> TopEdge = r -> MinY;
	}

	RenderWindow(win);

}

