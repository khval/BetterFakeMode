
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

APTR fake_LockBitMapTagList( struct BitMap * bitMap, struct TagItem * tagList)
{
	BOOL success = TRUE;
	struct TagItem *tag;

	if (tagList)
	{
		for (tag = tagList; (tag -> ti_Tag != TAG_DONE ) && ( tag -> ti_Tag != TAG_END); tag++)
		{
			switch (tag -> ti_Tag)
			{
				case LBM_BaseAddress:
					*((ULONG *) (tag -> ti_Data)) = bitMap -> Planes[0];
					break;
				case LBM_BytesPerRow:
					*((ULONG *) (tag -> ti_Data)) = bitMap -> BytesPerRow;
					break;
				case LBM_PixelFormat:
					*((ULONG *) (tag -> ti_Data)) = PIXF_CLUT;
					break;
				case LBM_PlanarYUVInfo:
					*((ULONG *) (tag -> ti_Data)) = 0;
					success = FALSE;
					break;
				case LBM_IsOnBoard:
					*((ULONG *) (tag -> ti_Data)) = FALSE;
					break;
			}
		}
	}
	return success ? (APTR) 0xFA8E : (APTR) 0x0000;
}