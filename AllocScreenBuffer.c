

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

extern APTR video_mutex;

// Yes I know, we should never alloc this buffer, but we must force it, because OS does allow it.

extern struct BitMap *_new_fake_bitmap(int Width,int Height, int Depth);
extern void _free_fake_bitmap( struct BitMap *bm );

struct ScreenBuffer * fake_AllocScreenBuffer ( struct Screen * s, struct BitMap * bm, ULONG flags)
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
	if (sb -> sb_DBufInfo) 
	{
		FreeVec( sb -> sb_DBufInfo );
		sb -> sb_DBufInfo = NULL;
	}

	if (sb -> sb_BitMap != s -> RastPort.BitMap)
	{
		 _free_fake_bitmap( sb -> sb_BitMap );
		sb -> sb_BitMap = NULL;
	}

	FreeVec( sb );
}

ULONG fake_ChangeScreenBuffer( struct Screen * s, struct ScreenBuffer * sb)
{
	struct Task *me;
	FPrintf( output,"fake_ChangeScreenBuffer\n");

	if (sb -> sb_BitMap)
	{
		MutexObtain(video_mutex);
		s -> RastPort.BitMap = sb -> sb_BitMap;
		s -> BitMap  = * (s -> RastPort.BitMap);
		MutexRelease(video_mutex);
	}

	me = FindTask(NULL);

	if (me)
	{
		if (sb->sb_DBufInfo)
		{
			if (sb->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort)
			{
				FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);
				Signal( me,  1L << sb->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort ->mp_SigBit);
			}
			else	FPrintf( output,"no ReplyPort\n");

			if (sb->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort)
			{
				FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);
				Signal( me,  1L << sb->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort ->mp_SigBit);
			}
			else	FPrintf( output,"no ReplyPort\n");	
		}
		else
		{
			FPrintf( output,"Missing sb_DBufInfoin ScreenBuffer\n");	
			getchar();
		}

		FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);
	}

	return TRUE;
}

