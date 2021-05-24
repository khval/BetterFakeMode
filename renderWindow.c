
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

#include "common.h"

extern struct Window *active_win;

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

extern void ReThinkGadgets(struct Window *w);

extern ULONG get_default_icon_size(struct RastPort *rp);

void RenderWindow(struct Window *win)
{
	int icon_s;
	int left_x = 0, right_x = 0;
	int x1,y1;
	int tmp_DetailPen;
	struct RastPort *rp = win -> RPort;
	
	x1 =  win -> Width;
	y1 =  win -> Height;

	tmp_DetailPen = win -> DetailPen;

	if (active_win == win)
	{
		SetAPen(rp,3);
	}
	else
	{
		SetAPen(rp,2);
	}

	box(rp,0,0,x1,y1);
	box(rp, win->BorderLeft, win -> BorderTop,x1 - win -> BorderRight,y1 - win -> BorderBottom );

	if (win -> FirstGadget) ReThinkGadgets( win );
	if (win -> FirstGadget) RenderGadgets( win -> RPort, win -> FirstGadget);

	if (win -> Title)
	{
		int y;
		icon_s = get_default_icon_size( win -> RPort );

		if (win->Flags & WFLG_CLOSEGADGET) left_x += icon_s;

		y = win -> BorderTop;
		y -= win -> RPort -> Font -> tf_YSize;
		y += win -> RPort -> Font -> tf_Baseline;

		Move(rp, left_x + 2, y );
		Text(rp,win -> Title, strlen(win->Title));
	}

	SetAPen(rp,tmp_DetailPen);
}

