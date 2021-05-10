

#include <stdio.h>
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"

extern void _free_fake_bitmap( struct BitMap *bm );

struct Window * fake_CloseWindow ( struct Window *w )
{
	if (w -> RPort)
	{
		if (w -> UserPort )
		{
			struct Message *msg;

			// empty msg Queue, make sure all messages are handled.

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
			_free_fake_bitmap( w -> RPort -> BitMap );
		}
		w -> RPort -> BitMap  = NULL;	


		FreeVec( w-> RPort);
		w -> RPort = NULL;
	}

	FreeVec( w );

	return NULL;
}


