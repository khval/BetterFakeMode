
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>
#include <proto/gadtools.h>

#include "init.h"


struct TextAttr MyFont =
{
	"topaz.font",
	TOPAZ_SIXTY,
	FS_NORMAL,
	FPF_ROMFONT
};

struct NewScreen myscr = 
	{
		0,0,
		320,
		200,
		8,
		0,1,
		(ULONG) NULL,
		CUSTOMSCREEN,
		&MyFont,
		"My Own Screen",
		NULL,
		NULL
	};

void set_colors( struct Screen *src )
{
	struct BitMap *bm = src -> RastPort.BitMap;
	int colors = 1L << bm -> Depth;
	int c;

	for (c=0;c<colors;c++) SetRGB32( &src -> ViewPort, 0xFFFFFFFF, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF );
}


void pplot( struct BitMap *bm, int x, int y)
{
	int bx = 7- (x&7) ;
	x /= 8;

	*(bm -> Planes[0] + x + y * bm -> BytesPerRow ) |= 1L<<bx;
}

bool open_extra_libs()
{
}

void close_extra_libs()
{
}

#define GAD_BUTTON 101

struct Gadget *CreateAllGadgets( struct Gadget *lastGadget, void *vi, UWORD topborder)
{
	struct NewGadget ng;
	struct Gadget *newGadget;

	ng.ng_VisualInfo = vi;
	ng.ng_GadgetID = GAD_BUTTON;
	ng.ng_LeftEdge = 10;
	ng.ng_TopEdge = 13+topborder;
	ng.ng_Width = 100;
	ng.ng_Height = 12;
	ng.ng_GadgetText = "Click Me";
	ng.ng_Flags = 0;

	newGadget = CreateGadget(BUTTON_KIND, lastGadget, &ng, TAG_DONE);
	if (!newGadget) Printf("failed to create gadget\n");

	return newGadget;
}

int main()
{
	struct Screen *src;
	struct RastPort *rp;

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	if (open_extra_libs() == FALSE)
	{
		close_extra_libs();
		close_libs();
		return 0;
	}

	src = OpenScreen( &myscr );

	if (src)
	{
		void *vi;
		struct Gadget *lastGadget;
		struct Gadget *glist = NULL;

		printf("%s:%d\n",__FUNCTION__,__LINE__);

		vi = GetVisualInfo( src , TAG_END );

      		lastGadget = CreateContext(&glist);

		if (!lastGadget) Printf("failed to create context\n");

		struct Window *win = OpenWindowTags( NULL, 
//			WA_Gadgets, glist,
			WA_Left, 10,
			WA_Top, 10,
			WA_Width, 200,
			WA_Height, 150,
			WA_Title, "Gadtools",
			WA_CustomScreen, src,
			TAG_END);

		if (win)
		{
			lastGadget = CreateAllGadgets( lastGadget, vi, win -> BorderTop);
			AddGList( (struct Window *) win,(struct Gadget *) glist,0,-1,NULL);
			RefreshGList( (void *) glist,(void *) win,0,-1); 

//			GT_RefreshWindow(win,NULL);
		}

		printf("Press enter to quit\n");
		getchar();

		if (win) CloseWindow( win );

		FreeGadgets(glist);

		CloseScreen( src ) ;
	}

	close_extra_libs();
	close_libs();
}

