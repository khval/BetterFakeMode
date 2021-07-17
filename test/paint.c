
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

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

					ReplyMsg( (struct Message *) m );

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

