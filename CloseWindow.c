

#include <stdio.h>
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"

struct Window * fake_CloseWindow ( struct Window *w )
{
	if (w -> RPort)
	{
		if (w -> RPort -> BitMap != w -> WScreen -> RastPort.BitMap)
		{
			printf("%s:%s:%d: this is maybe a bitmap we should free\n",
				__FILE__,__FUNCTION__,__LINE__);
		}
		w -> RPort -> BitMap  = NULL;	

		FreeVec( w-> RPort);
		w -> RPort = NULL;
	}

	FreeVec( w );

	return NULL;
}


