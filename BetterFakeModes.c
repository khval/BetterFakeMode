
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
APTR old_ppc_func_MoveScreen = NULL;
APTR old_ppc_func_ScreenPosition = NULL;
APTR old_ppc_func_SetWindowAttrsA = NULL;
APTR old_ppc_func_SetWindowAttr = NULL;

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
BPTR output = 0;

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


void fix_asl_tags( struct TagItem * tagList )
{
	struct TagItem *tag;

	if (tagList == NULL) 
	{
		FPrintf( output,"ASL is not using tags\n");
		return;
	}

	FPrintf( output,"ASL uses tags.... checking....\n");

	for (tag = tagList; tag -> ti_Tag != TAG_DONE; tag++)
	{
		switch (tag -> ti_Tag)
		{
			case ASLFR_Window:

				{
					struct Window *win = ((struct Window *) tag -> ti_Data);
					if (win)
					{
						if (is_fake_screen( win -> WScreen ))
						{
							FPrintf( output,"prohibit ASL from crash\n");
							tag -> ti_Data = (ULONG) NULL;
						}
					}
				}

				FPrintf( output,"ASLFR_Window\n");
				break;

			case ASLFR_PubScreenName:
				FPrintf( output,"ASLFR_PubScreenName\n");
				break;

			case ASLFR_Screen:
				if (is_fake_screen( (struct Screen *) tag -> ti_Data))
				{
					FPrintf( output,"prohibit ASL from crash\n");
					tag -> ti_Data = (ULONG) NULL;
				}
				break;
		}
	}
}
APTR ppc_func_OBSOLETE_AllocFileRequest(struct AslIFace *Self, uint32 type, struct TagItem *tags)
{
	FPrintf( output,"%s\n",__FUNCTION__);

	// need to look for ASLFR_Screen !!! in tags.
	fix_asl_tags( tags );

	return ((APTR (*) ( struct AslIFace *, uint32, struct TagItem *)) old_ppc_func_OBSOLETE_AllocFileRequest) (Self, type, tags);
}

APTR ppc_func_AllocAslRequest(struct AslIFace *Self, uint32 type, struct TagItem *tags)
{
	FPrintf( output,"%s -- type is %08lx (%ld)\n",__FUNCTION__,type,type);

	// need to look for ASLFR_Screen !!! in tags.
	fix_asl_tags( tags );

	return ((APTR (*) ( struct AslIFace *, uint32, struct TagItem *)) old_ppc_func_AllocAslRequest) (Self, type, tags);
}

BOOL set_patches( void )
{
	if (!IntuitionBase) return FALSE;

	Printf("libs is open, time to patch\n");

	IExec->Forbid();

	// ASL
	
	set_new_ppc_patch(Asl,OBSOLETE_AllocFileRequest);	// avoid fake screens for ASL...
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
	set_new_ppc_patch(Intuition,MoveScreen);			// monitor
	set_new_ppc_patch(Intuition,ScreenPosition);			// monitor
	set_new_ppc_patch(Intuition,SetWindowAttrsA);
	set_new_ppc_patch(Intuition,SetWindowAttr);

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
	undo_ppc_patch(Intuition,MoveScreen);
	undo_ppc_patch(Intuition,ScreenPosition);
	undo_ppc_patch(Intuition,SetWindowAttrsA);
	undo_ppc_patch(Intuition,SetWindowAttr);


	IExec->Permit();
}

bool quit = false;

ULONG host_sig;
struct Task *host_task;

extern void emuEngine();

const char *porg_name = "BetterFakeMode";
struct MsgPort *prog_port = NULL;

bool prog_has_started()
{
	struct MsgPort *have_port;

	IExec -> Forbid();
	have_port = IExec -> FindPort(porg_name);
	IExec -> Permit();

	if (have_port == NULL)
	{
		prog_port = (APTR) IExec -> AllocSysObjectTags(ASOT_PORT, 
					ASOPORT_Name, porg_name ,
					ASOPORT_CopyName, TRUE,
					TAG_DONE);
	}

	return prog_port ? true : false;
}

int main( void )
{
	struct Process *display_proc;
	BPTR disp_output;

	if (prog_has_started())
	{
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

		IExec -> FreeSysObject(ASOT_PORT, prog_port );
		prog_port = NULL;

	}

	return 0;
}


