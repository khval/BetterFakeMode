
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>
#include <exec/ports.h>

void box(struct RastPort *rp,int x0,int y0,int x1, int y1)
{
	Move(rp,x0,y0);
	Draw(rp,x1,y0);
	Draw(rp,x1,y1);
	Draw(rp,x0,y1);
	Draw(rp,x0,y0);	
}

void ClearBorder(struct Window *win)
{
	ULONG y0,y1;
	struct RastPort *rp = win -> RPort;

	y0 =0;
	y1 = win->BorderTop;

	RectFill( rp,0,y0,win -> Width,y1 );
	
	y0=y1;
	y1 = win -> Height - win -> BorderBottom;

	RectFill( rp,0,y0,win -> BorderLeft,y1 );
	RectFill( rp,win -> Width - win -> BorderRight,y0,win -> Width,y1 );

	y0=y1;
	y1 = win -> Height ;

	RectFill( rp,0,y0,win -> Width,y1 );
}

void RenderWindow(struct Window *win)
{
	int icon_w,icon_h;
	int left_x, right_x;
	int x1,y1;
	int tmp_DetailPen;
	struct RastPort *rp = win -> RPort;
	
	x1 =  win -> Width;
	y1 =  win -> Height;

	tmp_DetailPen = win -> DetailPen;

	SetAPen(rp,2);

	box(rp,0,0,x1,y1);
	box(rp, win->BorderLeft, win -> BorderTop,x1 - win -> BorderRight,y1 - win -> BorderBottom );

	icon_w = win -> BorderTop;
	icon_h = win -> BorderTop;

	left_x =0;

	if (win -> Flags & WFLG_CLOSEGADGET)
	{
		box(rp, 2, 2, icon_w - 2, icon_h - 2);
		left_x += icon_w;
	}

	if (win -> Flags & WFLG_SIZEGADGET)
	{
		ULONG x,y;
		x = win -> Width - icon_w;
		y = win -> Height - icon_h;

		box(rp, x+2, y+2, x+icon_w - 2, y+icon_h - 2);
	}
	
	if (win -> Title)
	{
		int y;

		y = win -> BorderTop;
		y -= win -> RPort -> Font -> tf_YSize;
		y += win -> RPort -> Font -> tf_Baseline;

		Move(rp, left_x + 2, y );
		Text(rp,win -> Title, strlen(win->Title));
	}

	SetAPen(rp,tmp_DetailPen);
}

