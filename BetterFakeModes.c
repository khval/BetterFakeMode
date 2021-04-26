
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
#include "spawn.h"

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

extern APTR video_mutex ;

struct Task *main_task = NULL;
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

struct Screen *lazy_screen_hack = NULL;

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
		IExec->MutexObtain(video_mutex);		// prevent screen from being drawn while we allocate screen.
		lazy_screen_hack = _new_fake_screen(newScreen -> Width,newScreen -> Height,newScreen -> Depth);
		IExec->MutexRelease(video_mutex);

		return lazy_screen_hack ;
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
		IExec->MutexObtain(video_mutex);		// prevent screen from being drawn while we free screen.
		_delete_fake_screen( screen );
		lazy_screen_hack = NULL;		// we only have one screen, so no worry... yet...
		IExec->MutexRelease(video_mutex);
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

	set_new_ppc_patch(OpenScreen);			// this points to OpenScreenTagList, but for now this is the one we hack.
//	set_new_ppc_patch(OpenScreenTags);		// don't need should point to OpenScreenTagList
	set_new_ppc_patch(OpenScreenTagList);
	set_new_ppc_patch(CloseScreen);
	
	return TRUE;
}

void undo_patches( void )
{
	undo_68k_patch(OpenScreen);
	undo_68k_patch(CloseScreen);

	undo_ppc_patch(OpenScreen);			// this points to OpenScreenTagList, but for now this is the one we hack.
//	undo_ppc_patch(OpenScreenTags);			// don't need should point to OpenScreenTagList
	undo_ppc_patch(OpenScreenTagList);
	undo_ppc_patch(CloseScreen);
}

bool quit = false;

extern unsigned char *bits2bytes[256];


void draw_bw( struct RastPort *rp, struct BitMap *bm )
{
	int x,y;
	unsigned char *end_of_line;
	unsigned char *ptr;
	unsigned char *b;
	unsigned char *be;
	 
	ptr = bm -> Planes[0];
	end_of_line = bm -> Planes[0] + bm -> BytesPerRow;

	for (y=0;y<bm -> Rows; y++)
	{
		x = 0;
		for ( ;ptr < end_of_line; ptr++ )
		{
			b = bits2bytes[ *ptr ];
			be = b+8;

			for (;b<be;b++,x++)		// bx = byte x
			{
				IGraphics->WritePixelColor(rp,  x,  y,  *b ? 0xFFFFFFFF : 0xFF000000 );
			}
		}
		end_of_line += bm -> BytesPerRow;	// end of next line...
	}
}


void dump_screen()
{
	struct Window *win = IIntuition -> OpenWindowTags( NULL, 
			WA_InnerWidth, 640,
			WA_InnerHeight, 480,
			TAG_END);


	if (!win) return ;

	do
	{
		IExec->MutexObtain(video_mutex);
		if (lazy_screen_hack)
		{
			draw_bw( win -> RPort, lazy_screen_hack -> RastPort.BitMap );
		}
		IExec->MutexRelease(video_mutex);

		IDOS -> Delay(1);
	} while (!quit);

	IIntuition -> CloseWindow( win );
}

int main( void )
{
	struct Process *display_proc;
	BPTR disp_output;

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

		Printf("-- press enter to quit\n");

		getchar();

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


