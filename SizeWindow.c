

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
	int miw = win -> MinWidth;
	int mih = win -> MinHeight;
	int maw = win -> MaxWidth;
	int mah = win -> MaxHeight;

	if (maw <=0) maw = win -> WScreen -> Width;
	if (mah <=0)  mah = win -> WScreen -> Height;
	if (maw < 30) maw = 30;
	if (mah < 30) mah = 30;
	if (miw < 30) miw = 30;
	if (mih < 30) mih = 30;

	w = win -> Width + dx;
	h = win -> Height + dy;

	if (w> maw ) w = maw;
	if (h> mah ) h = mah;

	if (w< miw) w = miw;
	if (h< mih) h = mih;

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

