
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "init.h"
#include "patch.h"
#include "common.h"

APTR old_68k_stub_OpenScreen = NULL;
APTR old_68k_stub_CloseScreen = NULL;

APTR old_ppc_func_OpenScreen = NULL;
APTR old_ppc_func_OpenScreenTags = NULL;
APTR old_ppc_func_OpenScreenTagList = NULL;
APTR old_ppc_func_CloseScreen = NULL;

#define _LVOOpenScreen	-198
#define _LVOCloseScreen	-66

#define Printf IDOS->Printf
#define FPrintf IDOS->FPrintf

struct Task *me;
BPTR output;

struct Screen *screens[100];
struct Screen **screens_end = screens + 100;

bool monitor = false;

int find_free_index()
{
	int i = 0;
	struct Screen **item;

	for (item = screens; item < screens_end ; item++,i++ )
	{
		if (*item == NULL) return i;
	}
	return -1;
}

void remove_screen_from_track(struct Screen *screen)
{
	struct Screen **item;

	for (item = screens; item < screens_end ; item++ )
	{
		if (*item == screen) 
		{
			item = NULL;
			return;
		}
	}
}


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

static struct Screen *ppc_func_OpenScreen( void *libBase, struct NewScreen *newScreen )
{
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

		return (( struct Screen *(*) ( void *, struct NewScreen * )) old_ppc_func_OpenScreen) (libBase, newScreen);
	}
	else
	{
		return _new_fake_screen(newScreen -> Width,newScreen -> Height,newScreen -> Depth);
	}
}

static void ppc_func_CloseScreen( void *libBase, struct Screen *screen )
{
	FPrintf( output,"CloseScreen\n");

	if (monitor)
	{
		((void (*) ( void *libBase, struct Screen *screen )) old_ppc_func_CloseScreen) (libBase, screen);
	}
	else
	{
		_delete_fake_screen( screen );
	}
}

static VOID stub_68k_OpenScreen_func( uint32 *regarray )
{
	regarray[REG68K_D0/4] =(uint32) ppc_func_OpenScreen( IntuitionBase, (struct NewScreen *) regarray[REG68K_A0/4] );
}

static VOID stub_68k_CloseScreen_func( uint32 *regarray )
{
	ppc_func_CloseScreen( IntuitionBase, (struct Screen *) regarray[REG68K_A0/4] );
}


STATIC CONST struct EmuTrap stub_68k_OpenScreen      = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *)) stub_68k_OpenScreen_func };
STATIC CONST struct EmuTrap stub_68k_CloseScreen      = { TRAPINST, TRAPTYPENR, (uint32 (*)(uint32 *)) stub_68k_CloseScreen_func };



BOOL set_patches( void )
{
	if (!IntuitionBase) return FALSE;

	Printf("libs is open, time to patch\n");

	set_new_68k_patch(OpenScreen);
	set_new_68k_patch(CloseScreen);

	set_new_ppc_patch(OpenScreen);
	set_new_ppc_patch(OpenScreenTags);
	set_new_ppc_patch(OpenScreenTagList);
	set_new_ppc_patch(CloseScreen);
	
	return TRUE;
}

void undo_patches( void )
{
	undo_68k_patch(OpenScreen);
	undo_68k_patch(CloseScreen);

	undo_ppc_patch(OpenScreen);
	undo_ppc_patch(OpenScreenTags);
	undo_ppc_patch(OpenScreenTagList);
	undo_ppc_patch(CloseScreen);
}


int main( void )
{
	me = IExec->FindTask(NULL);

	if (IS_PROCESS(me))
	{
		output = ((struct Process *) me) -> pr_COS;
	}

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	if (set_patches())
	{
		Printf("pacthes set\n");

		Printf("-- press enter to quit\n");

		getchar();

		undo_patches();
	}
	else
	{
		Printf("failed to patch\n");
	}

	close_libs();

	return 0;
}


