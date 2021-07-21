

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"

extern APTR video_mutex;

extern void _free_fake_bitmap( struct BitMap *bm );
extern void freeWinGadgets( struct Window *win );

struct Window * fake_CloseWindow ( struct Window *w )
{
	MutexObtain(video_mutex);

	if (w -> RPort)
	{
		if (w -> UserPort )
		{
			struct Message *msg;

			// empty msg Queue, make sure, make sure all messages are handled.

			msg = GetMsg( w-> UserPort );
			while ( msg )
			{
				ReplyMsg( msg );
				msg = GetMsg( w-> UserPort );
			}

			FreeSysObject( ASOT_PORT, w -> UserPort);
			w -> UserPort = NULL;
		}

		// Free Bitmap, if its not the same as screen bitmap.

		if (w -> RPort -> BitMap != w -> WScreen -> RastPort.BitMap)
		{
			if (w -> RPort -> BitMap -> pad == 0xFA8E)
			{
				_free_fake_bitmap( w -> RPort -> BitMap );
			}
			else
			{
				FreeBitMap( w -> RPort -> BitMap );
			}
		}
		w -> RPort -> BitMap  = NULL;	


		FreeVec( w-> RPort);
		w -> RPort = NULL;
	}

	if (w -> Title) free( w-> Title);
	w->Title= NULL; 

	if (w->ScreenTitle) free(w -> ScreenTitle);
	w -> ScreenTitle = NULL;

	freeWinGadgets( w );	// should only free intuition border gadgets.

	FreeVec( w );

	MutexRelease(video_mutex);

	return NULL;
}


