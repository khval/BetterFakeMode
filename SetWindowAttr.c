
#include <stdbool.h>
#include <stdlib.h>
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

extern APTR old_ppc_func_SetWindowAttrsA ;
extern APTR old_ppc_func_SetWindowAttr ;

extern void no_block_MoveWindow ( struct Window *win, LONG dx, LONG dy );
extern void no_block_SizeWindow ( struct Window *win, LONG dx, LONG dy );

LONG fake_SetWindowAttr (struct IntuitionIFace *Self, struct Window * win, ULONG attr, APTR data, ULONG size);


LONG fake_SetWindowAttrsA( struct IntuitionIFace *Self, struct Window *win, struct TagItem *tagList )
{
	LONG ret = 0;
	struct TagItem *tag;

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	for (tag = tagList; tag -> ti_Tag != TAG_DONE; tag++)
	{
		fake_SetWindowAttr (  Self, win, tag -> ti_Tag, (APTR) tag -> ti_Data, sizeof(ULONG));
	}
	return ret;
}

LONG fake_SetWindowAttr (struct IntuitionIFace *Self, struct Window * win, ULONG attr, APTR data, ULONG size)
{
	LONG ret = 0;
FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	MutexObtain(video_mutex);

	switch (attr)
	{
		case WA_Left:
				no_block_MoveWindow ( win, (LONG) data - win -> LeftEdge, 0 );
				break;
		case WA_Top:
				no_block_MoveWindow ( win, 0, (LONG) data - win -> TopEdge);
				break;

		case WA_Width:
				no_block_SizeWindow (  win, (LONG) data - win -> Width, 0 );
				break;

		case WA_Height:
				no_block_SizeWindow ( win, 0, (LONG) data - win -> Height );
				break;

		case WA_Title:

				break;

		default:
				ret = ((LONG (*) ( struct IntuitionIFace *, ULONG , APTR , ULONG )) old_ppc_func_SetWindowAttr) (Self, attr, data, size);
				break;
	}

	MutexRelease(video_mutex);
	return ret;
}




