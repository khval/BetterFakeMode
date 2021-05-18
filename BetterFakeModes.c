
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
#include <exec/emulation.h>
#include <exec/ports.h>

#include "init.h"
#include "patch.h"
#include "common.h"
#include "spawn.h"
#include "helper/screen.h"
#include "engine.h"

struct MsgPort *port;

APTR old_68k_stub_OpenScreen = NULL;
APTR old_68k_stub_CloseScreen = NULL;

APTR old_ppc_func_OpenScreenTagList = NULL;
APTR old_ppc_func_CloseScreen = NULL;

APTR old_ppc_func_OpenWindowTagList = NULL;
APTR old_ppc_func_CloseWindow = NULL;
APTR old_ppc_func_AllocScreenBuffer = NULL;
APTR old_ppc_func_FreeScreenBuffer = NULL;
APTR old_ppc_func_ChangeScreenBuffer = NULL;
APTR old_ppc_func_MoveWindow = NULL;
APTR old_ppc_func_SizeWindow = NULL;

#define _LVOOpenScreen	-198
#define _LVOCloseScreen	-66

#define Printf IDOS->Printf
#define FPrintf IDOS->FPrintf

extern APTR video_mutex ;

struct Task *main_task = NULL;
BPTR output;

bool monitor = false;

int num_of_open_screens = 0;

struct KownLgacyModes
{
	int w;
	int h;
};

struct KownLgacyModes LgacyModes[] =
{
	{320,200},	// NTSC, LOWRES
	{320,400},	// NTSC, LOWRES + interlaced
	{640,200},	// NTSC, HIRES
	{640,400},	// NTSC, HIRES + interlaced
	{320,256},	// PAL, LOWRES
	{320,512},	// PAL, LOWRES + interlaced
	{640,256},	// PAL, HIRES
	{640,512},	// PAL, HIRES + interlaced
	{-1,-1}
};


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

bool maybe_lagacy_mode(const struct NewScreen * newScreen)
{
	struct KownLgacyModes *lm;

	if (newScreen -> Depth>8) return false;

	for (lm = LgacyModes; lm -> w != -1; lm++ )
	{
		if ((newScreen -> Width == lm -> w) && (newScreen -> Height == lm -> h)) return true;
	}

	return false;
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

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	if ((newScreen) && (tagList == NULL))
	{
		is_lagacy = maybe_lagacy;
	}
	else 
	{
FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

		is_lagacy =legacy_in_tags(  tagList, maybe_lagacy );
	}

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	if (is_lagacy)
	{
		struct Screen *src;

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

		IExec->MutexObtain(video_mutex);		// prevent screen from being drawn while we allocate screen.

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

		src = _new_fake_OpenScreenTagList( newScreen, tagList );

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

		if (src) num_of_open_screens ++;

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

		IExec->MutexRelease(video_mutex);

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

		return src;
	}
	else
	{
		FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

		return (( struct Screen * (*)(struct IntuitionIFace *, const struct NewScreen * , const struct TagItem * )) old_ppc_func_OpenScreenTagList ) ( Self, newScreen, tagList );
	}

	return NULL;
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
		return  fake_OpenWindowTagList ( newWindow, tagList);
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

static void ppc_func_MoveWindow( struct IntuitionIFace *Self, struct Window *w, LONG dx, LONG dy  )
{
	FPrintf( output,"MoveWindow\n");

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
	FPrintf( output,"SizeWindow\n");

	if (is_fake_screen( w -> WScreen ))
	{
		fake_SizeWindow( w,dx,dy );
	}
	else
	{
		((void (*) ( struct IntuitionIFace *, struct Window *window, LONG x,  LONG y )) old_ppc_func_SizeWindow) (Self, w,dx,dy);
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


/*
static VOID stub_68k_OpenScreenTagList_func( uint32 *regarray )
{
	regarray[REG68K_D0/4] =(uint32) ppc_func_OpenScreen( IIntuition, (struct NewScreen *) regarray[REG68K_A0/4] );
}

static VOID stub_68k_CloseScreen_func( uint32 *regarray )
{
	ppc_func_CloseScreen( IIntuition, (struct Screen *) regarray[REG68K_A0/4] );
}

STATIC CONST struct EmuTrap stub_68k_OpenScreen      = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *)) stub_68k_OpenScreen_func };
STATIC CONST struct EmuTrap stub_68k_CloseScreen      = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *)) stub_68k_CloseScreen_func };
*/


BOOL set_patches( void )
{
	if (!IntuitionBase) return FALSE;

	Printf("libs is open, time to patch\n");

//	set_new_68k_patch(Intuition,OpenScreen);			// maybe not needed, as it will end up in PPC routines.
//	set_new_68k_patch(Intuition,CloseScreen);

	set_new_ppc_patch(Intuition,OpenScreenTagList);
	set_new_ppc_patch(Intuition,CloseScreen);
	set_new_ppc_patch(Intuition,AllocScreenBuffer);
	set_new_ppc_patch(Intuition,FreeScreenBuffer);
	set_new_ppc_patch(Intuition,ChangeScreenBuffer);
	set_new_ppc_patch(Intuition,MoveWindow);
	set_new_ppc_patch(Intuition,SizeWindow);

	set_new_ppc_patch(Intuition,OpenWindowTagList);
	set_new_ppc_patch(Intuition,CloseWindow);


	
	return TRUE;
}

void undo_patches( void )
{
//	undo_68k_patch(Intuition,OpenScreen);			// maybe not needed, as it will end up in PPC routines.
//	undo_68k_patch(Intuition,CloseScreen);

	undo_ppc_patch(Intuition,OpenScreenTagList);
	undo_ppc_patch(Intuition,CloseScreen);
	undo_ppc_patch(Intuition,AllocScreenBuffer);
	undo_ppc_patch(Intuition,FreeScreenBuffer);
	undo_ppc_patch(Intuition,ChangeScreenBuffer);
	undo_ppc_patch(Intuition,MoveWindow);
	undo_ppc_patch(Intuition,SizeWindow);

	undo_ppc_patch(Intuition,OpenWindowTagList);
	undo_ppc_patch(Intuition,CloseWindow);
}

bool quit = false;

ULONG host_sig;
struct Task *host_task;

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

	display_proc = spawn( dump_screen, "dump screen", disp_output );

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


