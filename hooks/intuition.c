

#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "hooks/intuition.h"
#include "helper/screen.h"
#include "fake/intuition.h"
#include "modeid.h"

extern BPTR output;
extern APTR video_mutex ;
extern bool monitor;
extern int num_of_open_screens;

struct Gadget *dupGadgat(struct Gadget *g);
ULONG attachGadget(struct Gadget *g, struct Window *win);
void RenderGadgets(struct RastPort *rp, struct Gadget *g);


VOID ppc_func_RefreshGList (struct IntuitionIFace *Self, struct Gadget * gadgets, struct Window * window, struct Requester * requester, WORD numGad)
{
	if (is_fake_screen( window -> WScreen ))
	{
		struct RastPort *rp = window -> RPort;
		ULONG tmp_DetailPen = window -> DetailPen;
		SetAPen(rp,3);
		RenderGadgets( window -> RPort , window -> FirstGadget);
		SetAPen(rp,tmp_DetailPen);
	}
	else
	{
		((void (*)( struct IntuitionIFace *, struct Gadget *, struct Window *, struct Requester *, WORD))
				old_ppc_func_RefreshGList) ( Self, gadgets, window, requester, numGad );
	}
}

UWORD ppc_func_AddGList(struct IntuitionIFace *Self, struct Window * window, struct Gadget * gadget, UWORD position, WORD numGad, struct Requester * requester)
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);

	if (is_fake_screen( window -> WScreen ))
	{
		ULONG ret = 0;
		struct Gadget *g = gadget ;
		while ((position)&&(g)) g=g->NextGadget;

		if (numGad == -1)
		{
			while (g) 
			{
				FPrintf( output, "%s:%ld Gadget %08lx\n",__FUNCTION__,__LINE__,g);

				ret = attachGadget(dupGadgat(g), window);
				g=g->NextGadget;
			}
		}
		else
		{
			while ((g)&&(numGad))
			{
				FPrintf( output, "%s:%ld Gadget %08x\n",__FUNCTION__,__LINE__,g);

				ret = attachGadget(dupGadgat(g), window);
				g=g->NextGadget;
				numGad--;
			}
		}
		return ret;
	}
	else
	{
		return ((UWORD (*)(struct IntuitionIFace *, struct Window * , struct Gadget * , UWORD , WORD , struct Requester * ))
				old_ppc_func_AddGList) ( Self, window, gadget, position,numGad,requester );
	}
}


VOID ppc_func_ScreenPosition(struct IntuitionIFace *Self, struct Screen * screen, ULONG flags, LONG x1, LONG y1, LONG x2, LONG y2)
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
	((VOID (*) (struct IntuitionIFace *, struct Screen * , ULONG , LONG , LONG , LONG , LONG )) old_ppc_func_ScreenPosition) (Self, screen, flags, x1,y1,x2,y2 );
}

VOID ppc_func_MoveScreen(struct IntuitionIFace *Self, struct Screen * screen, WORD dx, WORD dy)
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
	((VOID (*) (struct IntuitionIFace *, struct Screen * , WORD, WORD )) old_ppc_func_MoveScreen) (Self, screen, dx, dy );
}

void ppc_func_SetWindowTitles( struct IntuitionIFace *Self, struct Window *w, const char *winStr, const char *srcStr  )
{
	if (is_fake_screen( w -> WScreen ))
	{
		fake_SetWindowTitles( w,winStr, srcStr );
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Window *window, const char *, const char * )) old_ppc_func_SetWindowTitles) (Self, w, winStr, srcStr );
	}
}

struct ScreenBuffer * ppc_func_AllocScreenBuffer (struct IntuitionIFace *Self, struct Screen * sc, struct BitMap * bm, ULONG flags)
{
	FPrintf( output,"ppc_func_AllocScreenBuffer\n");

	if (is_fake_screen( sc ))
	{
		return fake_AllocScreenBuffer ( sc,  bm, flags);
	}
	else
	{
		return ((struct ScreenBuffer * (*) (struct IntuitionIFace *, struct Screen *, struct BitMap *, ULONG)) old_ppc_func_AllocScreenBuffer) (Self, sc, bm, flags);
	}
}

void ppc_func_SizeWindow( struct IntuitionIFace *Self, struct Window *w, LONG dx, LONG dy  )
{
	if (is_fake_screen( w -> WScreen ))
	{
		fake_SizeWindow( w,dx,dy );
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Window *window, LONG x,  LONG y )) old_ppc_func_SizeWindow) (Self, w,dx,dy);
	}
}

void ppc_func_MoveWindow( struct IntuitionIFace *Self, struct Window *w, LONG dx, LONG dy  )
{
	if (is_fake_screen( w -> WScreen ))
	{
		fake_MoveWindow( w,dx,dy );
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Window *window, LONG x,  LONG y )) old_ppc_func_MoveWindow) (Self, w,dx,dy);
	}
}

void ppc_func_ActivateWindow( struct IntuitionIFace *Self, struct Window *w )
{
	FPrintf( output,"ActivateWindow\n");

	if (is_fake_screen( w -> WScreen ))
	{
		fake_ActivateWindow( w );
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Window *window )) old_ppc_func_ActivateWindow) (Self, w);
	}
}

void ppc_func_CloseWindow( struct IntuitionIFace *Self, struct Window *w )
{
	FPrintf( output,"CloseWindow\n");

	if (is_fake_screen( w -> WScreen ))
	{
		fake_CloseWindow( w );
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Window *window )) old_ppc_func_CloseWindow) (Self, w);
	}
}

struct Window * ppc_func_OpenWindowTagList (struct IntuitionIFace *Self, const struct NewWindow * newWindow, const struct TagItem * tagList)
{
	bool has_a_fake_screen = false;

	FPrintf( output,"OpenWindow\n");

	if (newWindow)
	{
		if (is_fake_screen( newWindow -> Screen )) has_a_fake_screen = true;
	}
	else
	{
		const struct TagItem * tag;
		for (tag = tagList; tag -> ti_Tag != TAG_DONE; tag++)
		{
			switch (tag -> ti_Tag)
			{
				case WA_CustomScreen:
					if (is_fake_screen( (struct Screen *) tag -> ti_Data )) has_a_fake_screen = true;
					break; 
			}
		}
	}

	if (has_a_fake_screen)
	{
		struct Window *win;
		MutexObtain(video_mutex);
		win = fake_OpenWindowTagList ( newWindow, tagList);
		MutexRelease(video_mutex);
		return win;
	}
	else
	{
		return ((struct Window * (*) (struct IntuitionIFace *, const struct NewWindow *, const struct TagItem *)) old_ppc_func_OpenWindowTagList)	
				 (Self,newWindow,tagList);
	}
}

void ppc_func_CloseScreen( struct IntuitionIFace *Self, struct Screen *screen )
{
	FPrintf( output,"CloseScreen\n");

	if (monitor)
	{
		((void (*) ( struct IntuitionIFace *, struct Screen *)) old_ppc_func_CloseScreen) (Self, screen);
	}
	else
	{
		if (is_fake_screen( screen ))
		{
			MutexObtain(video_mutex);		// prevent screen from being drawn while we free screen.
			_delete_fake_screen( screen );

			allocatedScreen[ screens - screen ] = false;
			num_of_open_screens --;

			MutexRelease(video_mutex);
		}
		else
		{
			((void (*) ( struct IntuitionIFace *, struct Screen *)) old_ppc_func_CloseScreen) (Self, screen);
		}
	}
}

void ppc_func_ScreenToBack( struct IntuitionIFace *Self, struct Screen *screen )
{
	FPrintf( output,"ScreenToBack\n");


	if (is_fake_screen( screen ))
	{
		MutexObtain(video_mutex);	
		fake_screen_to_back( screen );
		MutexRelease(video_mutex);
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Screen *)) old_ppc_func_ScreenToBack) (Self, screen);
	}
}

void ppc_func_ScreenToFront( struct IntuitionIFace *Self, struct Screen *screen )
{
	FPrintf( output,"ScreenToFront\n");

	if (is_fake_screen( screen ))
	{
		MutexObtain(video_mutex);	
		fake_screen_to_front( screen );
		MutexRelease(video_mutex);
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Screen *)) old_ppc_func_ScreenToFront) (Self, screen);
	}
}

ULONG ppc_func_ChangeScreenBuffer(struct IntuitionIFace *Self, struct Screen * s, struct ScreenBuffer * sb)
{
	if (is_fake_screen(s))
	{
		return fake_ChangeScreenBuffer( s, sb );
	}
	else
	{
		return ((ULONG (*) ( struct IntuitionIFace *, struct Screen *,struct ScreenBuffer *)) old_ppc_func_ChangeScreenBuffer) (Self, s, sb);
	}
}

void ppc_func_FreeScreenBuffer( struct IntuitionIFace *Self, struct Screen *s, struct ScreenBuffer *sb )
{
	if (is_fake_screen( s ))
	{
		fake_FreeScreenBuffer( s, sb );
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Screen *,struct ScreenBuffer *)) old_ppc_func_FreeScreenBuffer) (Self, s, sb);
	}
}

struct Screen * ppc_func_OpenScreenTagList(struct IntuitionIFace *Self, const struct NewScreen * newScreen, const struct TagItem * tagList)
{
	int sw = 0,sh = 0,sd = 0;
	bool maybe_lagacy = false;
	bool is_lagacy = false;

	FPrintf( output, "OpenScreenTagList\n");

	if (newScreen)
	{
		show_newScreenInfo(newScreen);
		maybe_lagacy = maybe_lagacy_mode(newScreen);
		sw = newScreen -> Width;
		sh = newScreen -> Height;
		sd = newScreen -> Depth;
	}

	if ((newScreen) && (tagList == NULL))
	{
		FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
		is_lagacy = maybe_lagacy;
	}
	else 
	{
		struct modeT *mode =legacy_in_tags(  tagList, maybe_lagacy );

		FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);

		if ( mode )
		{
			if (mode == (struct modeT *) 0xFFFFFFFF)
			{
				FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
				is_lagacy = true;
			}
			else
			{
				sw = mode -> w;
				sh = mode -> h;
				sd = mode -> maxDepth;
				is_lagacy = true;
			}
		}
	}

	if (is_lagacy)
	{
		struct Screen *src;

		FPrintf( output, "Fake mode\n");

		MutexObtain(video_mutex);		// prevent screen from being drawn while we allocate screen.
		src = _new_fake_OpenScreenTagList( newScreen, tagList );

		if (src) num_of_open_screens ++;
		MutexRelease(video_mutex);

		return src;
	}
	else
	{

		FPrintf( output, "Real mode\n");

		return (( struct Screen * (*)(struct IntuitionIFace *, const struct NewScreen * , const struct TagItem * )) old_ppc_func_OpenScreenTagList ) ( Self, newScreen, tagList );
	}

	return NULL;
}

LONG ppc_func_SetWindowAttrsA (struct IntuitionIFace *Self, struct Window * win, struct TagItem * taglist)
{
	if (is_fake_screen( win -> WScreen ))
	{
		return fake_SetWindowAttrsA( Self, win, taglist );
	}
	else
	{
		return ((LONG (*) ( struct IntuitionIFace *, struct Window *,struct TagItem *)) old_ppc_func_SetWindowAttrsA) (Self, win, taglist);
	}

	return 0;
}

LONG ppc_func_SetWindowAttr (struct IntuitionIFace *Self, struct Window * win,ULONG attr, APTR data, ULONG size)
{
	if (is_fake_screen( win -> WScreen ))
	{
		return fake_SetWindowAttr( Self, win, attr, data, size );
	}
	else
	{
		return ((LONG (*) ( struct IntuitionIFace *, ULONG , APTR , ULONG )) old_ppc_func_SetWindowAttr) (Self, attr, data, size);
	}

	return 0;
}

