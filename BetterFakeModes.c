
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>
#include <proto/asl.h>
#include <exec/emulation.h>
#include <exec/ports.h>

#include "init.h"
#include "patch.h"
#include "common.h"
#include "spawn.h"
#include "modeid.h"
#include "helper/screen.h"
#include "engine.h"

struct MsgPort *port;

APTR old_68k_stub_OpenScreen = NULL;
APTR old_68k_stub_CloseScreen = NULL;

APTR old_ppc_func_OpenScreenTagList = NULL;
APTR old_ppc_func_CloseScreen = NULL;
APTR old_ppc_func_ScreenToFront = NULL;
APTR old_ppc_func_ScreenToBack = NULL;
APTR old_ppc_func_OpenWindowTagList = NULL;
APTR old_ppc_func_CloseWindow = NULL;
APTR old_ppc_func_AllocScreenBuffer = NULL;
APTR old_ppc_func_FreeScreenBuffer = NULL;
APTR old_ppc_func_ChangeScreenBuffer = NULL;
APTR old_ppc_func_MoveWindow = NULL;
APTR old_ppc_func_SizeWindow = NULL;
APTR old_ppc_func_SetWindowTitles = NULL;
APTR old_ppc_func_ActivateWindow = NULL;
APTR old_ppc_func_GetBitMapAttr = NULL;

// graphics

APTR old_ppc_func_LockBitMapTagList = NULL;
APTR old_ppc_func_UnlockBitMap = NULL;

// gadtools

APTR old_ppc_func_GetVisualInfo = NULL;
APTR old_ppc_func_FreeVisualInfo = NULL;
APTR old_ppc_func_GT_GetIMsg = NULL;
APTR old_ppc_func_GT_ReplyIMsg = NULL;
APTR old_ppc_func_CreateGadgetA = NULL;

APTR old_ppc_func_AddGList = NULL;
APTR old_ppc_func_RefreshGList = NULL;
APTR old_ppc_func_FreeGadgets = NULL;

// ASL

APTR old_ppc_func_AllocAslRequest = NULL;
APTR old_ppc_func_OBSOLETE_AllocFileRequest = NULL;

#include "hooks/gadtools.h"
#include "hooks/intuition.h"
#include "hooks/graphics.h"

#define _LVOOpenScreen	-198
#define _LVOCloseScreen	-66

#define Printf IDOS->Printf
#define FPrintf IDOS->FPrintf

extern APTR video_mutex ;

struct Task *main_task = NULL;
BPTR output;

bool monitor = false;

int num_of_open_screens = 0;

void show_newScreenInfo(const struct NewScreen * newScreen)
{
	char stdTXT[256];

	sprintf(stdTXT, "Width %d, Height %d, Depth %d, Type %d, CustomBitMap %08x, DefaultTitle %08x, Font %08x, ViewModes %08x",
		newScreen -> Width,
		newScreen -> Height,
		newScreen -> Depth,
		newScreen -> Type,
		newScreen -> CustomBitMap,
		newScreen -> DefaultTitle,
		newScreen -> Font,
		newScreen -> ViewModes );

	FPrintf( output, "%s\n",stdTXT);
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

		IExec->MutexObtain(video_mutex);		// prevent screen from being drawn while we allocate screen.
		src = _new_fake_OpenScreenTagList( newScreen, tagList );

		if (src) num_of_open_screens ++;
		IExec->MutexRelease(video_mutex);

		return src;
	}
	else
	{

		FPrintf( output, "Real mode\n");

		return (( struct Screen * (*)(struct IntuitionIFace *, const struct NewScreen * , const struct TagItem * )) old_ppc_func_OpenScreenTagList ) ( Self, newScreen, tagList );
	}

	return NULL;
}

APTR ppc_func_OBSOLETE_AllocFileRequest(struct AslIFace *Self, uint32 type, struct TagItem *tags)
{
	FPrintf( output,"%s\n",__FUNCTION__);

	// need to look for ASLFR_Screen !!! in tags.

	return ((APTR (*) ( struct AslIFace *, uint32, struct TagItem *)) old_ppc_func_OBSOLETE_AllocFileRequest) (Self, type, tags);
}

APTR ppc_func_AllocAslRequest(struct AslIFace *Self, uint32 type, struct TagItem *tags)
{
	FPrintf( output,"%s\n",__FUNCTION__);

	// need to look for ASLFR_Screen !!! in tags.

	return ((APTR (*) ( struct AslIFace *, uint32, struct TagItem *)) old_ppc_func_AllocAslRequest) (Self, type, tags);
}

ULONG ppc_func_ChangeScreenBuffer(struct IntuitionIFace *Self, struct Screen * s, struct ScreenBuffer * sb)
{
	if (is_fake_screen(s))
	{
		fake_ChangeScreenBuffer( s, sb );
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
		fake_FreeScreenBuffer( Self, s, sb );
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Screen *,struct ScreenBuffer *)) old_ppc_func_FreeScreenBuffer) (Self, s, sb);
	}
}

static void ppc_func_ScreenToFront( struct IntuitionIFace *Self, struct Screen *screen )
{
	FPrintf( output,"ScreenToFront\n");

	if (is_fake_screen( screen ))
	{
		IExec->MutexObtain(video_mutex);	
		fake_screen_to_front( screen );
		IExec->MutexRelease(video_mutex);
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Screen *)) old_ppc_func_ScreenToFront) (Self, screen);
	}
}

static void ppc_func_ScreenToBack( struct IntuitionIFace *Self, struct Screen *screen )
{
	FPrintf( output,"ScreenToBack\n");


	if (is_fake_screen( screen ))
	{
		IExec->MutexObtain(video_mutex);	
		fake_screen_to_back( screen );
		IExec->MutexRelease(video_mutex);
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Screen *)) old_ppc_func_ScreenToBack) (Self, screen);
	}
}

static void ppc_func_CloseScreen( struct IntuitionIFace *Self, struct Screen *screen )
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
			IExec->MutexObtain(video_mutex);		// prevent screen from being drawn while we free screen.
			_delete_fake_screen( screen );

			allocatedScreen[ screens - screen ] = false;
			num_of_open_screens --;

			IExec->MutexRelease(video_mutex);
		}
		else
		{
			((void (*) ( struct IntuitionIFace *, struct Screen *)) old_ppc_func_CloseScreen) (Self, screen);
		}
	}
}

static struct Window * ppc_func_OpenWindowTagList (struct IntuitionIFace *Self, const struct NewWindow * newWindow, const struct TagItem * tagList)
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
		IExec->MutexObtain(video_mutex);
		win = fake_OpenWindowTagList ( newWindow, tagList);
		IExec->MutexRelease(video_mutex);
		return win;
	}
	else
	{
		return ((struct Window * (*) (struct IntuitionIFace *, const struct NewWindow *, const struct TagItem *)) old_ppc_func_OpenWindowTagList)	
				 (Self,newWindow,tagList);
	}
}

static void ppc_func_CloseWindow( struct IntuitionIFace *Self, struct Window *w )
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

static void ppc_func_ActivateWindow( struct IntuitionIFace *Self, struct Window *w )
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

static void ppc_func_MoveWindow( struct IntuitionIFace *Self, struct Window *w, LONG dx, LONG dy  )
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

static void ppc_func_SizeWindow( struct IntuitionIFace *Self, struct Window *w, LONG dx, LONG dy  )
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

static void ppc_func_SetWindowTitles( struct IntuitionIFace *Self, struct Window *w, const char *winStr, const char *srcStr  )
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


BOOL set_patches( void )
{
	if (!IntuitionBase) return FALSE;

	Printf("libs is open, time to patch\n");

	IExec->Forbid();

	// ASL
	
	set_new_ppc_patch(Asl,OBSOLETE_AllocFileRequest);	// like monitor...
	set_new_ppc_patch(Asl,AllocAslRequest);				// avoid fake screens for ASL

	// Intuition

	set_new_ppc_patch(Intuition,OpenScreenTagList);
	set_new_ppc_patch(Intuition,CloseScreen);
	set_new_ppc_patch(Intuition,ScreenToFront);
	set_new_ppc_patch(Intuition,ScreenToBack);
	set_new_ppc_patch(Intuition,AllocScreenBuffer);
	set_new_ppc_patch(Intuition,FreeScreenBuffer);
	set_new_ppc_patch(Intuition,ChangeScreenBuffer);
	set_new_ppc_patch(Intuition,MoveWindow);
	set_new_ppc_patch(Intuition,SizeWindow);
	set_new_ppc_patch(Intuition,SetWindowTitles);
	set_new_ppc_patch(Intuition,ActivateWindow);
	set_new_ppc_patch(Intuition,OpenWindowTagList);
	set_new_ppc_patch(Intuition,CloseWindow);

#if patch_gadtools
	set_new_ppc_patch(Intuition,AddGList);
	set_new_ppc_patch(Intuition,RefreshGList);
#endif

	//  GadTools

#if patch_gadtools
	set_new_ppc_patch(GadTools,GetVisualInfo);
	set_new_ppc_patch(GadTools,FreeVisualInfo);
	set_new_ppc_patch(GadTools,CreateGadgetA);
	set_new_ppc_patch(GadTools,GT_GetIMsg);
	set_new_ppc_patch(GadTools,GT_ReplyIMsg);
	set_new_ppc_patch(GadTools,FreeGadgets);
#endif

	// Graphics

#if patch_GetBitMapAttr
	set_new_ppc_patch(Graphics,GetBitMapAttr);		// Fix bad values
#endif

#if patch_LockBitMap
	set_new_ppc_patch(Graphics,LockBitMapTagList);	// Lock does not work with fake bitmaps.
	set_new_ppc_patch(Graphics,UnlockBitMap);
#endif

	IExec->Permit();	

	return TRUE;
}

void undo_patches( void )
{

	IExec->Forbid();

	// undo Graphics

#if patch_GetBitMapAttr
	undo_ppc_patch(Graphics,GetBitMapAttr);
#endif

#if patch_LockBitMap
	undo_ppc_patch(Graphics,LockBitMapTagList);
	undo_ppc_patch(Graphics,UnlockBitMap);
#endif

	// ASL
	
	undo_ppc_patch(Asl,OBSOLETE_AllocFileRequest);
	undo_ppc_patch(Asl,AllocAslRequest);	

	// undo GadTools

#if patch_gadtools
	undo_ppc_patch(GadTools,GetVisualInfo);
	undo_ppc_patch(GadTools,FreeVisualInfo);
	undo_ppc_patch(GadTools,CreateGadgetA);
	undo_ppc_patch(GadTools,GT_GetIMsg);
	undo_ppc_patch(GadTools,GT_ReplyIMsg);
	undo_ppc_patch(GadTools,FreeGadgets);
#endif

	// Intuition

#if patch_gadtools
	undo_ppc_patch(Intuition,AddGList);
	undo_ppc_patch(Intuition,RefreshGList);
#endif

	undo_ppc_patch(Intuition,OpenScreenTagList);
	undo_ppc_patch(Intuition,CloseScreen);
	undo_ppc_patch(Intuition,ScreenToFront);
	undo_ppc_patch(Intuition,ScreenToBack);
	undo_ppc_patch(Intuition,AllocScreenBuffer);
	undo_ppc_patch(Intuition,FreeScreenBuffer);
	undo_ppc_patch(Intuition,ChangeScreenBuffer);
	undo_ppc_patch(Intuition,MoveWindow);
	undo_ppc_patch(Intuition,SizeWindow);
	undo_ppc_patch(Intuition,SetWindowTitles);
	undo_ppc_patch(Intuition,ActivateWindow);
	undo_ppc_patch(Intuition,OpenWindowTagList);
	undo_ppc_patch(Intuition,CloseWindow);
	IExec->Permit();
}

bool quit = false;

ULONG host_sig;
struct Task *host_task;

extern void emuEngine();

int main( void )
{
	struct Process *display_proc;
	BPTR disp_output;

	printf("sizeof(screens) %d\n",sizeof(screens));

	main_task = IExec->FindTask(NULL);

	if (IS_PROCESS(main_task))
	{
		output = ((struct Process *) main_task) -> pr_COS;
	}

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	host_sig = IExec -> AllocSignal(-1);
	host_task = IExec -> FindTask(NULL);

//	disp_output = IDOS -> Open("CON:660/32/320/200/display debug", MODE_NEWFILE );
	disp_output = IDOS -> Open("NIL:", MODE_NEWFILE );

	display_proc = spawn( emuEngine, "emuEngine", disp_output );

	if (set_patches())
	{

		for(;;) 
		{
			IExec -> Wait( 1L << host_sig  | SIGBREAKF_CTRL_C);

			if (num_of_open_screens)
			{
				Printf("you can't quit before all fake screens are closed\n");
			}
			else 
			{
				Printf("Quit..\n");
				break;
			}
		}

		undo_patches();
	}
	else
	{
		Printf("failed to patch\n");
	}

	quit = true;
	wait_spawns();

	IExec->FreeSignal( host_sig );

	close_libs();

	return 0;
}


