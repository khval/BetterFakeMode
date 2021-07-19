
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

extern void box(struct RastPort *rp,int x0,int y0,int x1, int y1);

extern BPTR output;

void RenderGadget(struct RastPort *rp, struct Gadget *g)
{
	LONG x0,y0,x1,y1;

	x0=g->LeftEdge;
	y0=g->TopEdge;
	x1=x0+g->Width;
	y1=y0+g->Height;

	box(rp, x0,y0,x1,y1);

	FPrintf( output, "gadget %ld,%ld,%ld,%ld\n", x0,y0,x1,y1);
}

void RenderGadgets(struct RastPort *rp, struct Gadget *g)
{
	for ( ; g ; g = g -> NextGadget )
	{
		if (g -> Activation) RenderGadget(rp, g);
	}
}

void ReThinkGadgets(struct Window *w)
{
	ULONG left_edge = 0;
	ULONG right_edge = 0;
	struct Gadget *g;
	struct Gadget *dragbar = NULL;

	for (g = w -> FirstGadget; g ; g = g -> NextGadget )
	{
		switch ( g -> Flags )
		{
			case GTYP_CLOSE:
				g -> TopEdge = 0;
				g -> LeftEdge = 0;
				left_edge += g -> Width;
				break;

			case GTYP_WDRAGGING:
				dragbar = g;
				break;

			case GTYP_WDEPTH:
				g -> TopEdge = 0;
				g -> LeftEdge = w -> Width - g -> Width;
				right_edge += g -> Width;
				break;

			case GTYP_WZOOM:
				switch (w -> Flags & (WFLG_SIZEGADGET|WFLG_DEPTHGADGET))
				{
					case WFLG_SIZEGADGET:
						g -> TopEdge = 0;
						g -> LeftEdge = w -> Width - g -> Width;
						break;

					case (WFLG_SIZEGADGET|WFLG_DEPTHGADGET):
						g -> TopEdge = 0;
						g -> LeftEdge = w -> Width - (g -> Width * 2);
						break;
				}
				right_edge += g -> Width;
				break;

			case GTYP_SIZING:
				g -> TopEdge = w -> Height - g -> Height;
				g -> LeftEdge = w -> Width - g -> Width;
				break;

			default:
				break;
		}
	}

	if (dragbar)
	{
		dragbar -> LeftEdge = left_edge;
		dragbar -> Width = w -> Width - left_edge - right_edge;
	}

}

