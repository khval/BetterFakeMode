
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
		2,
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


void show_layer_Info(struct Layer_Info *li)
{
	printf(
		" top_layer: %08x\n"
		"resPtr1: %08x\n"
		"resPtr2: %08x\n"
		"FreeClipRects: %08x\n"
		"PrivateReserve3: %d\n"
		"ClipRectPool: %08x\n"
   	 	"Flags: %08x\n"
		"res_count: %d\n"
		"LockLayersCount: %d\n"
		"PrivateReserve5: %d\n"
		"BlankHook: %08x\n"
		"Extension: %08x\n",

			li -> top_layer,
			li -> resPtr1,
			li -> resPtr2,
			li -> FreeClipRects,
			li -> PrivateReserve3,
			li -> ClipRectPool,
   		 	li -> Flags,
			li -> res_count,
			li -> LockLayersCount,
			li -> PrivateReserve5,
			li -> BlankHook,
			li -> Extension);
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

	src = OpenScreen( &myscr );

	if (src)
	{
		struct DrawInfo *di;

		di = GetScreenDrawInfo( src );
		if (di)
		{
			printf("we have DrawInfo\n");

			FreeScreenDrawInfo( src, di );
		}


		struct Window *win = OpenWindowTags( NULL, 
			WA_Left, 10,
			WA_Top, 10,
			WA_Width, 100,
			WA_Height, 100,
			WA_CustomScreen, src,
			TAG_END);

		printf("Window open? %s -- its at: %08x\n", win ? "Yes" : "No",win);
		printf("src -> FirstWindow: %08x\n", src -> FirstWindow);
		printf("src -> RastPort: %08x\n",&src -> RastPort);
		printf("src -> RastPort -> BitMap: %08x\n",src->RastPort.BitMap);

		printf("src -> ViewPort.DWidth: %d\n",src->ViewPort.DWidth);
		printf("src -> ViewPort.DHeight: %d\n",src->ViewPort.DHeight);
		printf("src -> ViewPort.DxOffset: %d\n",src->ViewPort.DxOffset);
		printf("src -> ViewPort.DyOffset: %d\n",src->ViewPort.DyOffset);
		printf("src -> ViewPort.Modes: %08x\n",src->ViewPort.Modes);

		show_layer_Info( &src -> LayerInfo );


		if (win)
		{
			ActivateWindow( win );

			Delay(5);

			MoveWindow( win, 66,77);

			printf("win -> LeftEdge %d\n",win->LeftEdge);
			printf("win -> TopEdge %d\n",win->TopEdge);
			printf("win -> Width: %d\n",win->Width);
			printf("win -> Height: %d\n",win->Height);

			printf("win -> RastPort: %08x\n",win->RPort);

			if (win->RPort)
			{
				struct RastPort *rp = win->RPort;

				printf("win -> RastPort -> Layer: %08x\n", rp -> Layer);
				printf("win -> RastPort -> BitMap: %08x\n", rp -> BitMap);
				printf("win -> RastPort -> AreaPtrn: %08x\n", rp -> AreaPtrn);
				printf("win -> RastPort -> TmpRas: %08x\n", rp -> TmpRas);
				printf("win -> RastPort -> AreaInfo: %08x\n", rp -> AreaInfo);
				printf("win -> RastPort -> GelsInfo: %08x\n", rp -> GelsInfo);
			}

			printf("win -> UserPort: %08x\n",win->UserPort);
		}

		printf("Press enter to quit\n");
		getchar();

		if (win) CloseWindow( win );

		CloseScreen( src ) ;
	}

	close_libs();
}

