// OSCopper.e
// Native graphics example using OS friendly copperlist

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
#include "../debug.h"

bool initScreen()
{
	screen=OpenScreenTags(NULL,
					SA_Title,"OS Copper",
//					SA_Pens,[-1]:INT,
					SA_Depth,4,
					TAG_END);

	if (!screen) return false;

	window=OpenWindowTags(NULL,
					WA_IDCMP,IDCMP_MOUSEBUTTONS,
					WA_Flags,WFLG_NOCAREREFRESH |
							WFLG_ACTIVATE |
							WFLG_BORDERLESS |
							WFLG_BACKDROP,
					WA_CustomScreen,screen,
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
		ULONG backrgb;
		int i;
		int linestart=screen -> BarHeight+1;
		int lines=screen -> Height-linestart;
		int width=screen -> Width;
		

		show_screen( screen );
		show_win(window);

		viewport=ViewPortAddress(window);
		struct RastPort *rport=window -> RPort;
		backrgb = ((ULONG *) viewport -> ColorMap -> ColorTable)[0];

		if (rport)
		{
			if (rport -> BitMap)
			{
				Box(rport,0,linestart,width-1,screen -> Height-1,1);
			}
			else Printf("wtf... no bitmap...\n");
		}
		else Printf("wtf... no rastport...\n");

#if 1
		
	// 5 instructions per line, and 3 extra at the end.
		
		CINIT(myucoplist,(lines*5) + 3 );
		
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
#endif

#if 0
		RethinkDisplay();
#endif
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

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

	ret = main_prog();

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

	close_libs();

	return 0;
}

