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

#include "common.h"

struct RastPort *rport;

int i;
ULONG linestart,lines,backrgb;
ULONG x,y;

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

#ifdef __amigaos3__
	myucoplist=AllocVec(sizeof(struct UCopList),MEMF_PUBLIC | MEMF_CLEAR);
#endif

#ifdef __amigaos4__
	myucoplist=AllocVecTags(sizeof(struct UCopList),
				AVT_Type, MEMF_SHARED, 
				AVT_Alignment,  16, 
				AVT_ClearWithValue, 0,
				TAG_DONE);
#endif

	if (!myucoplist) return false;

	return true;
}


void errors()
{
	if (!screen) Printf("Unable to open screen.\n");
	if (!window) Printf("Unable to open window.\n");
	if (!myucoplist) Printf("Unable to allocate myucoplist memory.\n");
}

int main_prog()
{
	if (initScreen())
	{
		viewport=ViewPortAddress(window);
		rport=window -> RPort;

		lines = window -> RPort -> BitMap -> Rows;

		backrgb = ((ULONG *) viewport -> ColorMap -> ColorTable)[0];

		SetColour(screen,0,0,0,0);
		SetColour(screen,1,255,255,255);
	
		SetAPen(rport,1);

		for (y=0;y<256;y+=64) for (x=0;x<192;x+=64) RectFill(rport,x,y,x+31,y+31);
	
		CINIT(myucoplist,(lines*5)+3);
	
		for (i=linestart;i<lines;i++)
		{
			CWAIT(myucoplist,i,0);
	  		CMOVEA(myucoplist,BPLCON3,0);
	  		CMOVEA(myucoplist,COLOR(1),(i-linestart) & 0xFFF);
	  		CMOVEA(myucoplist,BPLCON3,0x200);
	  		CMOVEA(myucoplist,COLOR(1),(0xFFF-i) & 0xFFF);
		}
	
		CWAIT(myucoplist,i,0);
		CMOVEA(myucoplist,COLOR(1),backrgb);
		CEND(myucoplist);
	
		Forbid();
	 	viewport -> UCopIns=myucoplist;
		Permit();

		if (viewport -> UCopIns)
		{
			struct CopList  *cl;
			struct CopIns *c;

			dumpUCopList( viewport -> UCopIns );

			cl = viewport -> UCopIns -> FirstCopList;
			if ( cl )
			{
				dumpCopList( cl );

				c = cl -> CopIns;
				dumpCopIns( c, cl -> Count );
			}
		}

		RethinkDisplay();

		WaitLeftMouse(window);
	}
	else
	{
		errors();		
	}

	closeDown();

	return 0;
}

int main()
{
	int ret;

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	ret = main_prog();

	close_libs();

	return 0;
}

