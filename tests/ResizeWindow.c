
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
		int n;
		struct DrawInfo *di;
		struct Window *win[2];

		di = GetScreenDrawInfo( src );
		if (di)
		{
			printf("we have DrawInfo\n");

			FreeScreenDrawInfo( src, di );
		}

/*
#define WFLG_SIZEGADGET       include sizing system-gadget?     
#define WFLG_DRAGBAR          include dragging system-gadget?   
#define WFLG_DEPTHGADGET      include depth arrangement gadget? 
#define WFLG_CLOSEGADGET      include close-box system-gadget?  
#define WFLG_SIZEBRIGHT       size gadget uses right border     
#define WFLG_SIZEBBOTTOM      size gadget uses bottom border   
*/

		win[0] = OpenWindowTags( NULL, 
			WA_Title, "Back Win",
			WA_Flags, WFLG_SIZEBBOTTOM,
			WA_Left, 10, WA_Top, 10,
			WA_Width, 100, WA_Height, 100,
			WA_MinWidth, 100, WA_MinHeight, 100,
			WA_MaxWidth, 320, WA_MaxHeight, 200,
			WA_CustomScreen, src, TAG_END);

		win[1] = OpenWindowTags( NULL, 
			WA_IDCMP, IDCMP_CLOSEWINDOW,
			WA_Flags, WFLG_DRAGBAR | WFLG_CLOSEGADGET | WFLG_SIZEGADGET,
			WA_Title, "Front Win",
			WA_Left, 50, WA_Top, 50,
			WA_Width, 100, WA_Height, 100,
			WA_MinWidth, 100, WA_MinHeight, 100,
			WA_MaxWidth, 320, WA_MaxHeight, 200,
			WA_CustomScreen, src, TAG_END);


		printf("Window open? %s -- its at: %08x\n", win ? "Yes" : "No",win);

		show_layer_Info( &src -> LayerInfo );

		if (win[0])
		{
			int x,y;

			ActivateWindow( win[0] );

			SetAPen( win[0] -> RPort, 2);

			x = rand()%200;
			y = rand()%200;
			Move( win[0] -> RPort,x,y );

			for (n=0;n<100;n++)
			{
				x = rand()%200;
				y = rand()%200;
				Draw( win[0] -> RPort,x,y );
				printf("xy %d,%d\n",x,y);
			}

			for (n=0;n<10;n++)
			{
				Delay(20);
				SizeWindow( win[0], 20, 20 );
				Delay(20);
				SizeWindow( win[0], -20, -20 );
			}
		}

		win_mask = 0;
		for (n=0;n<2;n++)
		{
			win_mask |= win[n] -> UserPort ? 1 << win[n] -> UserPort ->mp_SigBit : 0;
		}

		do
		{
			Printf("Waiting for signal\n");
			ULONG sig = Wait( win_mask | SIGBREAKF_CTRL_C);

			if (sig & SIGBREAKF_CTRL_C)	break;

			for (n=0;n<2;n++)
			{
				if (sig  & (1 << win[n] -> UserPort ->mp_SigBit) )
				{
					m = (struct IntuiMessage *) GetMsg( win[n] -> UserPort );

					while (m)
					{
						switch (m -> Class)
						{
							case IDCMP_CLOSEWINDOW:
								quit = true;
								break;
						}

						ReplyMsg( (struct Message *) m );
						m = (struct IntuiMessage *) GetMsg( win[n] -> UserPort );
					}
				}
			}

		} while (!quit);

		Printf("Ready to quit...\n");

		for (n=0;n<2;n++)
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

