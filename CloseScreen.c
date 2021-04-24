
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"

void _cleanup_colormap( struct ColorMap *cm )
{
	if (cm -> ColorTable)	
	{
		IExec->FreeVec( cm -> ColorTable );
		cm -> ColorTable = NULL;
	}
}

void _cleanup_fake_ViewPort( struct ViewPort *vp )
{
	if (vp -> ColorMap)
	{
		_cleanup_colormap( vp -> ColorMap );
		IExec -> FreeVec(vp -> ColorMap);
		vp -> ColorMap = NULL;
	}
}

void _delete_fake_screen( struct Screen *s )
{
	int d;
	int depth;
	struct BitMap *bm;
	
	_cleanup_fake_ViewPort( &s -> ViewPort );

	bm = s -> RastPort.BitMap;
	if (bm)
	{
		depth = bm -> Depth;
		for (d=0;d<depth;d++)
		{
			IExec -> FreeVec( bm -> Planes[d] );
			bm -> Planes[d] = NULL;
		}

		IExec -> FreeVec(bm);
	}
	IExec -> FreeVec(s);
}


