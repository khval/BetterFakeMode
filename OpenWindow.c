

#include <stdio.h>
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"

extern void RenderWindow(struct Window *win);

struct Window * fake_OpenWindowTagList ( const struct NewWindow * nw, const struct TagItem * tagList)
{
	struct Window *win = NULL;
	const struct TagItem * tag;

	win =  new_struct( Window );

	if (win == NULL) return NULL;

	// Defaults....

	win -> BorderLeft = 2;
	win -> BorderTop = 0;
	win -> BorderRight = 2;
	win -> BorderBottom = 0;

	if (nw)
	{
    		win->DetailPen=nw->DetailPen;
		win->BlockPen=nw->BlockPen;
		win->IDCMPFlags=nw->IDCMPFlags;
		win->Flags=nw->Flags;
		win->FirstGadget=nw->FirstGadget;
		win->CheckMark=nw->CheckMark;
		win->Title=nw->Title;
		win->WScreen=nw->Screen;
		win->MinWidth=nw->MinWidth;
		win->MinHeight=nw->MinHeight;
		win->MaxWidth=nw->MaxWidth;
		win->MaxHeight=nw->MaxHeight;
	}

	for (tag = tagList; tag -> ti_Tag != TAG_DONE; tag++)
	{
		switch (tag -> ti_Tag)
		{
			case WA_DetailPen: 
    				win->DetailPen=tag -> ti_Data; break; 

			case WA_BlockPen: 
				win->BlockPen=tag -> ti_Data; break; 

			case WA_IDCMP:
				win->IDCMPFlags=tag -> ti_Data; break; 

			case WA_Title:
				win->Title= (char *) tag -> ti_Data; break; 

			case WA_ScreenTitle:
				win->ScreenTitle= (char *) tag -> ti_Data; break; 

			case WA_CustomScreen: 
				win -> WScreen = (struct Screen *) tag -> ti_Data; break; 

			case WA_Left: 
				win -> LeftEdge = tag -> ti_Data; break; 

			case WA_Top: 
				win -> TopEdge = tag -> ti_Data; break; 

			case WA_Width:
				win -> Width = tag -> ti_Data; break; 

			case WA_Height:
				win -> Height = tag -> ti_Data; break; 

			case WA_MinWidth:
				win->MinWidth= tag -> ti_Data; break; 

			case WA_MinHeight:
				win->MinHeight= tag -> ti_Data; break; 

			case WA_MaxWidth:
				win->MaxWidth= tag -> ti_Data; break; 

			case WA_MaxHeight:
				win->MaxHeight= tag -> ti_Data; break; 
				break;

		}
	}

	if (win -> IDCMPFlags)
	{
		win -> UserPort = (APTR) AllocSysObjectTags(ASOT_PORT, TAG_DONE);
	}

	win -> RPort = new_struct( RastPort );

	if (win->WScreen)
	{
		struct Window *oldWin = win -> WScreen -> FirstWindow;
		win -> NextWindow = oldWin;
		win -> WScreen -> FirstWindow = win;

		if (win -> RPort)
		{
			InitRastPort( win -> RPort );
			win -> RPort -> BitMap = win -> WScreen -> RastPort.BitMap;
			SetFont( &win -> RPort, default_font );

			if (win -> Title)
			{
				win -> BorderTop = win -> RPort -> Font -> tf_YSize + 4;
			}

			RenderWindow(win);
		}

	}

	return win;
}
