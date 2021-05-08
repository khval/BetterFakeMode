

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"



// Yes I know, we should never alloc this buffer, but we must force it, because OS does allow it.

extern struct BitMap *_new_fake_bitmap(int Width,int Height, int Depth);
extern void _free_fake_bitmap( struct BitMap *bm );

struct ScreenBuffer * fake_AllocScreenBuffer (struct IntuitionIFace *Self, struct Screen * s, struct BitMap * bm, ULONG flags)
{
	struct ScreenBuffer *sb;

	FPrintf( output,"fake_AllocScreenBuffer\n");

	sb =  new_struct( ScreenBuffer );

	if (sb)
	{
		sb -> sb_DBufInfo = new_struct( DBufInfo) ;

		switch (flags)
		{
			case SB_SCREEN_BITMAP:
				sb -> sb_BitMap = s -> RastPort.BitMap;
				break;

			case SB_COPY_BITMAP:
				{
					struct BitMap *b = s -> RastPort.BitMap;
					sb -> sb_BitMap = _new_fake_bitmap( b -> BytesPerRow * 16, b -> Rows, b -> Depth);
				}
				break;
		}
	}

	return sb;
}

void fake_FreeScreenBuffer( struct Screen *s, struct ScreenBuffer *sb )
{
	if (sb -> sb_DBufInfo) FreeVec( sb -> sb_DBufInfo );
	if (sb -> sb_BitMap) _free_fake_bitmap( sb -> sb_BitMap );
	FreeVec( sb );
}


