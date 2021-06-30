

#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "hooks/graphics.h"
#include "helper/screen.h"

ULONG ppc_func_GetBitMapAttr (struct GraphicsIFace *Self, struct BitMap * bitMap, ULONG value)
{
	if ( bitMap -> pad == 0xFA8E)
	{
		return fake_GetBitMapAttr ( bitMap, value);
	}
	else
	{
		return ((ULONG (*) (struct GraphicsIFace *,  struct BitMap *, ULONG)) old_ppc_func_GetBitMapAttr) (Self, bitMap, value);
	}
}

APTR  ppc_func_LockBitMapTagList(struct GraphicsIFace *Self, struct BitMap * bitMap, struct TagItem * tags)
{
	if ( bitMap -> pad == 0xFA8E)
	{
		return fake_LockBitMapTagList ( bitMap, tags );
	}
	else
	{
		return ((APTR (*) (struct GraphicsIFace *,  struct BitMap *, struct TagItem *)) old_ppc_func_LockBitMapTagList) (Self, bitMap, tags);
	}
}

void  ppc_func_UnlockBitMap(struct GraphicsIFace *Self, APTR lock)
{
	if ( lock != (APTR) 0xFA8E)
	{
		((void (*) (struct GraphicsIFace *, APTR)) old_ppc_func_UnlockBitMap) (Self, lock);
	}
}

