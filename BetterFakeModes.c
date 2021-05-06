
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

struct MsgPort *port;

APTR old_68k_stub_OpenScreen = NULL;
APTR old_68k_stub_CloseScreen = NULL;

APTR old_ppc_func_OpenScreen = NULL;
APTR old_ppc_func_OpenScreenTags = NULL;
APTR old_ppc_func_OpenScreenTagList = NULL;
APTR old_ppc_func_CloseScreen = NULL;

APTR old_ppc_func_OpenWindowTagList = NULL;
APTR old_ppc_func_CloseWindow = NULL;


#define _LVOOpenScreen	-198
#define _LVOCloseScreen	-66

#define Printf IDOS->Printf
#define FPrintf IDOS->FPrintf

extern APTR video_mutex ;

struct Task *main_task = NULL;
BPTR output;

char *screens_end_ptr = (char *) screens +sizeof(screens);

bool monitor = false;

int num_of_open_screens = 0;

struct Screen * ppc_func_OpenScreenTagList(struct IntuitionIFace *Self, const struct NewScreen * newScreen, const struct TagItem * tagList)
{
	FPrintf( output,"OpenScreenTagList()\n");

	if (monitor)
	{
		return (( struct Screen * (*)(struct IntuitionIFace *, const struct NewScreen * , const struct TagItem * )) old_ppc_func_OpenScreenTagList ) ( Self, newScreen, tagList );
	}
	else
	{

	}

	return NULL;
}

struct Screen * ppc_func_OpenScreenTags(struct IntuitionIFace *Self, const struct NewScreen * newScreen, ...)
{
	va_list ap;
	ULONG tag;
	ULONG value;

	FPrintf( output, "OpenScreenTags()\n");
	
	va_start(ap, newScreen);

	do
	{
		tag = va_arg(ap, ULONG);


		switch (tag)
		{
			case SA_Left:
			case SA_Top:
			case SA_Width:
			case SA_Height:
			case SA_Depth:
			case SA_DetailPen:
			case SA_BlockPen:
			case SA_Title:
			case SA_Colors:
			case SA_ErrorCode:
			case SA_Font:
			case SA_SysFont:
			case SA_Type:
			case SA_BitMap:
			case SA_PubName:
			case SA_PubSig:
			case SA_PubTask:
			case SA_DisplayID:
			case SA_DClip:
			case SA_Overscan:
			case SA_Obsolete1:
			case SA_ShowTitle:
			case SA_Behind:
			case SA_Quiet:
			case SA_AutoScroll:
			case SA_Pens:
			case SA_FullPalette:
			case SA_ColorMapEntries:
			case SA_Parent:
			case SA_Draggable:
			case SA_Exclusive:
			case SA_SharePens:
			case SA_BackFill:
			case SA_Interleaved:
			case SA_Colors32:
			case SA_VideoControl:
			case SA_FrontChild:
			case SA_BackChild:
			case SA_LikeWorkbench:
			case SA_Reserved:
			case SA_MinimizeISG:
			case SA_OffScreenDragging:
			case SA_Reserved2:
			case SA_ActiveWindow:
			case SA_MaxWindowBox:
			case SA_Reserved3:
			case SA_Compositing:
			case SA_WindowDropShadows:

				value = va_arg(ap, ULONG);

				Printf("%08x: %08x\n",tag,value);

				break;
		}

	} while (tag != TAG_END );

	va_end(ap);


	return NULL;
}


static struct Screen *ppc_func_OpenScreen( struct IntuitionIFace *Self, struct NewScreen *newScreen )
{
	struct Screen *src;
	char stdTXT[256];

	FPrintf( output, "OpenScreen\n");

	sprintf(stdTXT, "Width %ld, Height %ld, Depth %ld, Type %ld, CustomBitMap %08lx, DefaultTitle %08lx, Font %08lx, ViewModes %08lx",
		newScreen -> Width,
		newScreen -> Height,
		newScreen -> Depth,
		newScreen -> Type,
		newScreen -> CustomBitMap,
		newScreen -> DefaultTitle,
		newScreen -> Font,
		newScreen -> ViewModes );

	FPrintf( output, "%s\n",stdTXT);

	if (monitor)
	{
		// on AmigaOS4, this will result in OpenScreenTagList() being called.

		return (( struct Screen *(*) ( struct IntuitionIFace *, struct NewScreen * )) old_ppc_func_OpenScreen) (Self, newScreen);
	}
	else
	{
		IExec->MutexObtain(video_mutex);		// prevent screen from being drawn while we allocate screen.
		src = _new_fake_screen(newScreen -> Width,newScreen -> Height,newScreen -> Depth);
		if (src) num_of_open_screens ++;

		IExec->MutexRelease(video_mutex);

		return src;
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


static VOID stub_68k_OpenScreen_func( uint32 *regarray )
{
	regarray[REG68K_D0/4] =(uint32) ppc_func_OpenScreen( IIntuition, (struct NewScreen *) regarray[REG68K_A0/4] );
}

static VOID stub_68k_CloseScreen_func( uint32 *regarray )
{
	ppc_func_CloseScreen( IIntuition, (struct Screen *) regarray[REG68K_A0/4] );
}


STATIC CONST struct EmuTrap stub_68k_OpenScreen      = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *)) stub_68k_OpenScreen_func };
STATIC CONST struct EmuTrap stub_68k_CloseScreen      = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *)) stub_68k_CloseScreen_func };



BOOL set_patches( void )
{
	if (!IntuitionBase) return FALSE;

	Printf("libs is open, time to patch\n");

	set_new_68k_patch(Intuition,OpenScreen);			// maybe not needed, as it will end up in PPC routines.
	set_new_68k_patch(Intuition,CloseScreen);

	set_new_ppc_patch(Intuition,OpenScreen);			// this points to OpenScreenTagList, but for now this is the one we hack.
	set_new_ppc_patch(Intuition,OpenScreenTagList);
	set_new_ppc_patch(Intuition,CloseScreen);

	set_new_ppc_patch(Intuition,OpenWindowTagList);
	set_new_ppc_patch(Intuition,CloseWindow);
	
	return TRUE;
}

void undo_patches( void )
{
	undo_68k_patch(Intuition,OpenScreen);			// maybe not needed, as it will end up in PPC routines.
	undo_68k_patch(Intuition,CloseScreen);

	undo_ppc_patch(Intuition,OpenScreen);			// this points to OpenScreenTagList, but for now this is the one we hack.
	undo_ppc_patch(Intuition,OpenScreenTagList);
	undo_ppc_patch(Intuition,CloseScreen);

	undo_ppc_patch(Intuition,OpenWindowTagList);
	undo_ppc_patch(Intuition,CloseWindow);
}

bool quit = false;


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

	disp_output = IDOS -> Open("CON:660/32/320/200/display debug", MODE_NEWFILE );

	display_proc = spawn( dump_screen, "dump screen", disp_output );

	if (set_patches())
	{
		Printf("pacthes set\n");

		Printf("If console window is behind, hold CTRL+LAMIGA to drag window...\n");
		Printf("-- press enter to quit\n");


		for(;;) 
		{
			getchar();

			if (num_of_open_screens)
			{
				printf("you can't quit before all fake screens are closed\n");
			}
			else break;
		}

		undo_patches();
	}
	else
	{
		Printf("failed to patch\n");
	}

	quit = true;
	wait_spawns();

	close_libs();

	return 0;
}


