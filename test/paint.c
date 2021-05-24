
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>
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

void show_ArieaInfo(struct AreaInfo *ai)
{
	printf(
		"AreaInfo-> *VctrTbl: %08x\n"
    		"AreaInfo-> *VctrPtr: %08x\n"
		"AreaInfo-> *FlagTbl: %08x\n"
		"AreaInfo-> *FlagPtr: %08x\n"
		"AreaInfo-> Count: %d\n"
		"AreaInfo-> MaxCount: %d\n"
		"AreaInfo-> FirstX: %d\n"
		"AreaInfo-> FirstY: %d\n",
			ai->VctrTbl,
			ai->VctrPtr,
			ai->FlagTbl,
			ai->FlagPtr,
			ai->Count,
			ai->MaxCount,
			ai->FirstX,
			ai->FirstY);
}

void show_screen( struct Screen *src )
{
	printf("src -> FirstWindow: %08x\n", src -> FirstWindow);
	printf("src -> RastPort: %08x\n",&src -> RastPort);
	printf("src -> RastPort -> BitMap: %08x\n",src->RastPort.BitMap);

	printf("src -> ViewPort.DWidth: %d\n",src->ViewPort.DWidth);
	printf("src -> ViewPort.DHeight: %d\n",src->ViewPort.DHeight);
	printf("src -> ViewPort.DxOffset: %d\n",src->ViewPort.DxOffset);
	printf("src -> ViewPort.DyOffset: %d\n",src->ViewPort.DyOffset);
	printf("src -> ViewPort.Modes: %08x\n",src->ViewPort.Modes);
}

void show_rp(struct RastPort *rp)
{
	printf("win -> RastPort -> Layer: %08x\n", rp -> Layer);
	printf("win -> RastPort -> BitMap: %08x\n", rp -> BitMap);
	printf("win -> RastPort -> AreaPtrn: %08x\n", rp -> AreaPtrn);
	printf("win -> RastPort -> TmpRas: %08x\n", rp -> TmpRas);
	printf("win -> RastPort -> AreaInfo: %08x\n", rp -> AreaInfo);
	printf("win -> RastPort -> GelsInfo: %08x\n", rp -> GelsInfo);

	printf("win -> RastPort -> cp_x: %d\n", rp -> cp_x);
	printf("win -> RastPort -> cp_y: %d\n", rp -> cp_y);
	printf("win -> RastPort -> PenWidth: %d\n", rp -> PenWidth);
	printf("win -> RastPort -> PenHeight: %d\n", rp -> PenHeight);
}

void show_win( struct Window *win )
{
	printf("win -> LeftEdge %d\n",win->LeftEdge);
	printf("win -> TopEdge %d\n",win->TopEdge);
	printf("win -> Width: %d\n",win->Width);
	printf("win -> Height: %d\n",win->Height);
	printf("win -> RastPort: %08x\n",win->RPort);
	printf("win -> UserPort: %08x\n",win->UserPort);
	printf("win -> Flags: %08x\n",win->Flags);
}

int main()
{
	struct Screen *src;
	struct RastPort *rp;
	bool quit = false;
	ULONG win_mask;
	struct IntuiMessage *m;

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	src = OpenScreen( &myscr );

	if (src)
	{
		char info[30];
		int n;
		struct DrawInfo *di;
		struct Window *win[2];

		di = GetScreenDrawInfo( src );
		if (di)
		{
			printf("we have DrawInfo\n");

			FreeScreenDrawInfo( src, di );
		}

		win[0] = OpenWindowTags( NULL, 
			WA_IDCMP, 
				IDCMP_CLOSEWINDOW |
				IDCMP_MOUSEMOVE |
				IDCMP_INTUITICKS |
				IDCMP_MOUSEBUTTONS,
			WA_Flags, 
				WFLG_DRAGBAR | 
				WFLG_CLOSEGADGET ,
			WA_Title, "Front Win",
			WA_Left, 0, WA_Top, 20,
			WA_Width, 200, WA_Height, 150,
			WA_ReportMouse, true,
			WA_CustomScreen, src, TAG_END);


		printf("Window open? %s -- its at: %08x\n", win ? "Yes" : "No",win);

		show_layer_Info( &src -> LayerInfo );

		win_mask = win[0] -> UserPort ? 1 << win[0] -> UserPort ->mp_SigBit : 0;

		do
		{
			Printf("Waiting for signal\n");
			ULONG sig = Wait( win_mask | SIGBREAKF_CTRL_C);
			BOOL painting = false;

			if (sig & SIGBREAKF_CTRL_C)	break;

			if (sig  & win_mask )
			{
				struct RastPort *rp;

				rp = win[0] -> RPort;

				m = (struct IntuiMessage *) GetMsg( win[0] -> UserPort );

				while (m)
				{

					SetAPen( rp, 2);
					Move(rp, 10,20);
					sprintf(info, "Class: %08x",m->Class);
					Text(rp, info, strlen(info) );

					switch (m -> Class)
					{
						case IDCMP_MOUSEBUTTONS:

							painting = ( m->Code & IECODE_UP_PREFIX) ? false : true;
							break;

						case IDCMP_MOUSEMOVE:

							Printf("Mouse %ld,%ld\n",m->MouseX, m->MouseY);

				//			if (painting)
							{
								SetAPen( rp, 2);
								WritePixel( rp, m -> MouseX, m-> MouseY );
							}
							break;

						case IDCMP_CLOSEWINDOW:
							quit = true;
							break;
					}

					ReplyMsg( m );

					m = (struct IntuiMessage *) GetMsg( win[0] -> UserPort );
				}
			}

		} while (!quit);

		Printf("Ready to quit...\n");

		for (n=0;n<1;n++)
		{
			if (win[n]) 
			{
				printf("Window %d\n",n);
				show_win( win[n] );

				Printf("close window %d\n",n);
				CloseWindow( win[n] );
			}
		}
		CloseScreen( src ) ;
	}

	close_libs();
}

