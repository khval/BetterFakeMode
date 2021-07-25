// CopperCheck
// Native graphics example using OS friendly copperlist
// Displays checker pattern then scrolls each line indivdiually

#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include	<graphics/gfxbase.h>
#include	<graphics/gfxmacros.h>
#include	<graphics/copper.h>
#include	<graphics/view.h>
#include	<hardware/custom.h>
#include	<intuition/intuition.h>
#include	<intuition/screens.h>
#include	<exec/memory.h>

typedef void * PTR;

struct Screen *screen=NULL;
struct Window *window=NULL;
PTR myucoplist=NULL;
struct ViewPort *viewport = NULL;
struct RastPort *rport;

int i;
ULONG linestart,lines,backrgb;
ULONG x,y;

#define COLOR 0x180
#define BPLCON3 0x106

#define SetColour(src,a,r,g,b) SetRGB32( &(src -> ViewPort), a*0x01010101, r*0x01010101,g*0x01010101,b*0x01010101 )

bool initScreen()
{
	struct ColorSpec colors[]={
			{0,0xF,0x0,0x0},
			{-1,0x0,0x0,0x0}
		};

	screen=OpenScreenTags(NULL,
		SA_Quiet,TRUE,
		SA_Depth,1,
		SA_Width,320,
		SA_Height,256,
		SA_Colors,colors,
		SA_Type,CUSTOMSCREEN,
		TAG_END);

	if (!screen) return false;

	window=OpenWindowTags(NULL,
				WA_IDCMP,IDCMP_MOUSEBUTTONS,
				WA_Flags,WFLG_NOCAREREFRESH |
					WFLG_ACTIVATE |
					WFLG_BORDERLESS |
					WFLG_BACKDROP |
					WFLG_RMBTRAP,
				WA_CustomScreen,screen,
				WA_Left,0,
				WA_Top,0,
				WA_Width,320,
				WA_Height,256,
				TAG_END);

	if (!window) return false;

	myucoplist=AllocVecTags(sizeof(struct UCopList),MEMF_PUBLIC | MEMF_CLEAR);

	if (!myucoplist) return false;

	return true;
}

void closeDown()
{
	if (window)
	{
		if (viewport -> UCopIns)
		{
			FreeVPortCopLists(viewport);
			RemakeDisplay();
		}
		CloseWindow(window);
		window = NULL;
	}

	if (screen)
	{
		CloseScreen(screen);
		screen = NULL;	
	}

	if (myucoplist)
	{
		FreeVec(myucoplist);
		myucoplist = NULL;

	}
}

void errors()
{
	if (!screen) Printf("Unable to open screen.\n");
	if (!window) Printf("Unable to open window.\n");
	if (!myucoplist) Printf("Unable to allocate myucoplist memory.\n");
}

int main()
{
	if (initScreen())
	{
		viewport=ViewPortAddress(window);
		rport=window -> RPort;
		backrgb=Int(viewport -> ColorMap -> ColorTable);
		SetStdRast(rport);
		SetColour(screen,0,0,0,0);
		SetColour(screen,1,255,255,255);
	
		for (y=0;y<256;y+=64) for (x=0;x<192;x+=64) Box(x,y,32,32,1);
	
		CINIT(myucoplist,lines*4);
	
		for (i=linestart;i<lines;i++)
		{
			CWAIT(myucoplist,i,0);
	  		CMOVEA(myucoplist,BPLCON3,0);
	  		CMOVEA(myucoplist,COLOR+2,(i-linestart) & 0xFFF);
	  		CMOVEA(myucoplist,BPLCON3,0x200);
	  		CMOVEA(myucoplist,COLOR+2,(0xFFF-i) & 0xFFF);
		}
	
		CWAIT(myucoplist,i,0);
		CMOVEA(myucoplist,COLOR+2,backrgb);
		CEND(myucoplist);
	
		Forbid();
	 	viewport -> UCopIns=myucoplist;
		Permit();
		RethinkDisplay();

		{
			BOOL running = TRUE;
			struct IntuiMessage *msg;
			ULONG class, code;
			ULONG win_mask = 1 << window -> UserPort ->mp_SigBit ;
	
		 	do
			{
				ULONG sig = Wait( win_mask | SIGBREAKF_CTRL_C);

				do
				{
					msg = (struct IntuiMessage *) GetMsg( window );

					if (msg)
					{
						code = msg -> Code;
						class = msg -> Class;
						switch (class)
						{
							case IDCMP_MOUSEBUTTONS :
								running = false;
								break;
						}
						ReplyMsg( (struct Message *) msg);
					}
				} while (msg);

			} while (running);

			ModifyIDCMP( window, 0L );  /* tell Intuition to stop sending more messages */

			while (msg = (struct IntuiMessage *) GetMsg( window ))
			{
				ReplyMsg( (struct Message *) msg);
			}
		}
	}
	else
	{
		errors();		
	}

	closeDown();

	return 0;
}
