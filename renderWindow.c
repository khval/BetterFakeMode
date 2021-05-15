
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

void RenderWindow(struct Window *win)
{
	int x0,y0,x1,y1;
	int tmp_DetailPen;
	struct RastPort *rp = win -> RPort;
	
	x0 = 0;
	y0 = 0;
	x1 = x0 + win -> Width;
	y1 = y0 + win -> Height;

	tmp_DetailPen = win -> DetailPen;

	SetAPen(rp,2);

	box(rp,x0,y0,x1,y1);
	box(rp,x0 + win->BorderLeft,y0 + win -> BorderTop,x1 - win -> BorderRight,y1 - win -> BorderBottom );
	
	if (win -> Title)
	{
		int y;

		y = win -> BorderTop;
		y -= win -> RPort -> Font -> tf_YSize;
		y += win -> RPort -> Font -> tf_Baseline;

		Move(rp, 2, y );
		Text(rp,win -> Title, strlen(win->Title));
	}

	SetAPen(rp,tmp_DetailPen);
}

