

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"

extern void RenderWindow(struct Window *win);

extern void dump_tags( const struct TagItem * tagList , struct win_info **nameList);

extern void addWinGadget(struct Window *win, ULONG icon_s, ULONG flags);

ULONG get_default_icon_size(struct RastPort *rp)
{
	ULONG icon_s = rp -> Font -> tf_YSize + 4;
	if (icon_s < 5) icon_s = 5;
	return icon_s;
}

void addWindowGadgets( struct Window *win )
{
	ULONG icon_s = get_default_icon_size( win -> RPort );

	if (win -> Flags)
	{
		if (win->Flags & WFLG_CLOSEGADGET)
			addWinGadget( win, icon_s, GTYP_CLOSE );

		if (win->Flags & WFLG_DRAGBAR)
			addWinGadget( win, icon_s, GTYP_WDRAGGING );

		if (win->Flags & WFLG_DEPTHGADGET)
			addWinGadget( win, icon_s, GTYP_WDEPTH );

		if (win->Flags & WFLG_SIZEGADGET)
			addWinGadget( win, icon_s, GTYP_SIZING );
	}
}

void attach_to_screen( struct Window *win)
{
	struct Window *oldWin = win -> WScreen -> FirstWindow;
	win -> NextWindow = oldWin;
	win -> WScreen -> FirstWindow = win;
}

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
	win -> FirstGadget = NULL;
	win -> Title = NULL;
	win -> ScreenTitle = NULL;

	if (nw)
	{
    	win->DetailPen=nw->DetailPen;
		win->BlockPen=nw->BlockPen;
		win->IDCMPFlags=nw->IDCMPFlags;
		win->Flags=nw->Flags;
		win->FirstGadget=nw->FirstGadget;
		win->CheckMark=nw->CheckMark;
		win->Title=nw -> Title ? strdup(nw->Title) : NULL;
		win->WScreen=nw->Screen;
		win->MinWidth=nw->MinWidth;
		win->MinHeight=nw->MinHeight;
		win->MaxWidth=nw->MaxWidth;
		win->MaxHeight=nw->MaxHeight;
	}

	if (tagList)
	{
//		dump_tags( tagList , win_info_tags);

		for (tag = tagList; (tag -> ti_Tag != TAG_DONE ) && ( tag -> ti_Tag != TAG_END); tag++)
		{
			switch (tag -> ti_Tag)
			{
				case WA_DetailPen: 
   	 				win->DetailPen=tag -> ti_Data; break; 

				case WA_BlockPen: 
					win->BlockPen=tag -> ti_Data; break; 

				case WA_IDCMP:
					win->IDCMPFlags=tag -> ti_Data; break; 

				case WA_Flags:
					win->Flags=tag -> ti_Data;
					break; 

				case WA_Title:

					if (tag -> ti_Data)
					{
						if (win -> Title) free( win-> Title);
						win->Title= strdup((char *) tag -> ti_Data); 
					}
					break; 

				case WA_ScreenTitle:

					if (tag -> ti_Data)
					{
						if (win->ScreenTitle) free(win -> ScreenTitle);
						win -> ScreenTitle = strdup((char *) tag -> ti_Data);
					}
					 break; 
		
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
	}

	if (win -> IDCMPFlags)
	{
		char name[100];
		sprintf(name,"FakeWindow%08x",win);

		win -> UserPort = (APTR) AllocSysObjectTags(ASOT_PORT, 
							ASOPORT_Name, name,
							ASOPORT_CopyName, TRUE,
							TAG_DONE);

		FPrintf( output, "UserPort: %08x\n", win -> UserPort);
	}

	win -> RPort = new_struct( RastPort );

	if (win->WScreen)
	{
		if (win -> Flags & WFLG_BACKDROP)
		{
			FPrintf( output, "Window is a backdrop\n");

			win -> Width = win->WScreen->Width;
			win -> Height = win->WScreen->Height;
		}

		// limit to screen size...

		if (win->Width > win->WScreen->Width) win->Width = win ->WScreen -> Width;
		if (win->Height > win->WScreen->Height) win->Height = win-> WScreen -> Height;

		// move the window into the screen.
	
		if (win -> TopEdge<0) win -> TopEdge = 0;
		if (win -> LeftEdge<0) win -> LeftEdge = 0;

		if ( (win->TopEdge+win->Height) > win->WScreen->Height)
			win->TopEdge = win->WScreen->Height - win -> Height;

		if ( (win->LeftEdge+win->Width) > win->WScreen->Width)
			win->LeftEdge = win->WScreen->Width - win -> Width;

		if (win -> RPort)
		{
			ULONG i;
			ULONG icon_s;

			InitRastPort( win -> RPort );
			win -> RPort -> BitMap = win -> WScreen -> RastPort.BitMap;

			SetFont( win -> RPort, default_font );

			icon_s = get_default_icon_size( win -> RPort );

			addWindowGadgets( win );

			if (win -> Title)
			{
				win -> BorderTop = icon_s;
			}

			if (win -> Flags & WFLG_SIZEGADGET)
			{
				if (win -> Flags & WFLG_SIZEBBOTTOM)
				{
					win -> BorderBottom = icon_s;
				}
				else
				{
					win -> BorderRight = icon_s;
				}
			}

			win -> GZZWidth = win -> Width - win -> BorderLeft - win -> BorderRight;
			win -> GZZHeight = win -> Height - win -> BorderTop - win -> BorderBottom;

			i = win -> WScreen - screens;	// get the index..

			win -> RPort -> Layer = CreateLayer( LayerInfos[i], 
				LAYA_BitMap,  win -> RPort -> BitMap ,
				LAYA_MinX, win -> LeftEdge,
				LAYA_MinY, win -> TopEdge,
				LAYA_MaxX, win -> LeftEdge + win -> Width ,
				LAYA_MaxY, win -> TopEdge + win -> Height ,
				TAG_END);

			win -> IFont = default_font;
			win -> WLayer = win -> RPort -> Layer;

			attach_to_screen( win );

			if (win -> Flags & WFLG_ACTIVATE)
			{
				no_block_ActivateWindow(win);
			}
			else
			{			
				RenderWindow(win);
			}
		}
	}

	return win;
}
