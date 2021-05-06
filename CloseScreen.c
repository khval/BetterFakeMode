
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"

void _cleanup_colormap( struct ColorMap *cm )
{
	if (cm -> ColorTable)	
	{
		FreeVec( cm -> ColorTable );
		cm -> ColorTable = NULL;
	}
}

void _cleanup_fake_ViewPort( struct ViewPort *vp )
{
	if (vp -> ColorMap)
	{
		_cleanup_colormap( vp -> ColorMap );
		FreeVec(vp -> ColorMap);
		vp -> ColorMap = NULL;
	}

	if (vp->RasInfo)
	{
		FreeVec(vp -> RasInfo);
		vp -> RasInfo = NULL;
	}
}

void _free_fake_bitmap( struct BitMap *bm )
{
	// memory is allocated as large chunk.

	FreeVec( bm -> Planes[0] );
	bm -> Planes[0] = NULL;
	FreeVec(bm);
}

void _cleanup_fake_screen( struct Screen *s )
{

	struct BitMap *bm;
	
	_cleanup_fake_ViewPort( &s -> ViewPort );

	bm = s -> RastPort.BitMap;
	if (bm)
	{
#if use_fake_bitmap == 1
		_free_fake_bitmap( bm );
#else
		FreeBitMap(bm);
#endif
	}
}

void _delete_fake_screen( struct Screen *s )
{
	_cleanup_fake_screen( s );
	remove_screen_from_list( s );
}


