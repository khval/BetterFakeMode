
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

struct Library			*GadToolsBase = NULL;
struct GadToolsIFace	*IGadTools = NULL;

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
	if ( ! open_lib( "gadtools.library", 51L , "main", 1, &GadToolsBase, (struct Interface **) &IGadTools ) ) return FALSE;
	return TRUE;
}

void close_extra_libs()
{
	close_lib(GadTools);
}

#define GAD_BUTTON 101

struct Gadget *CreateAllGadgets( struct Gadget **glistptr, void *vi, UWORD topborder)
{
	struct NewGadget ng;
	struct Gadget *gad;

	ng.ng_LeftEdge = 10;
	ng.ng_TopEdge = 19+topborder;
	ng.ng_Width = 100;
	ng.ng_Height = 12;
	ng.ng_GadgetText = "ClickMe";
	ng.ng_GadgetID = GAD_BUTTON;
	ng.ng_Flags = 0;

	gad = CreateGadget(BUTTON_KIND, gad, &ng,
			TAG_DONE);

	return(gad);
}

int main()
{
	struct Screen *src;
	struct RastPort *rp;
	UWORD topborder;


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
		struct Gadget *gad;
		struct Gadget *glist = NULL;

		printf("%s:%d\n",__FUNCTION__,__LINE__);

		topborder = src->WBorTop + (src->Font->ta_YSize + 1);

		printf("%s:%d\n",__FUNCTION__,__LINE__);

      		gad = CreateContext(&glist);

		vi = GetVisualInfo( src , TAG_END );

		CreateAllGadgets(&glist, vi, topborder);

		struct Window *win = OpenWindowTags( NULL, 
			WA_Gadgets, glist,
			WA_Left, 10,
			WA_Top, 10,
			WA_Width, 200,
			WA_Height, 150,
			WA_Title, "Gadtools",
			WA_CustomScreen, src,
			TAG_END);

		if (win)
		{
			GT_RefreshWindow(win,NULL);
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

