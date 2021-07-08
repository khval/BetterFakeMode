
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"

extern APTR video_mutex;





void _cleanup_fake_ViewPort( struct ViewPort *vp )
{
	if (vp -> ColorMap)
	{
		FreeColorMap( vp -> ColorMap );
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
	struct RastPort *rp;
	struct BitMap **bm;
	
	_cleanup_fake_ViewPort( &s -> ViewPort );

	rp = &s -> RastPort;

	if (rp -> Font)
	{
		if (rp -> Font != default_font)
		{
			CloseFont( rp -> Font );
		}
		rp -> Font = NULL;
	}


	if (rp -> BitMap)
	{
#if use_fake_bitmap == 1

		if ( rp -> BitMap -> pad == 0xFA8E)	// check, if its custum bitmap or not, (tell me how to do it corrent.)
		{
			_free_fake_bitmap(rp -> BitMap );
			rp -> BitMap = NULL;
		}
#else
		// **** WARNING NEED TO CHECK FOR CUSTUM BITMAP.

		FreeBitMap(rp -> BitMap);
		rp -> BitMap = NULL;
#endif
	}
}

void _delete_fake_screen( struct Screen *s )
{
	_cleanup_fake_screen( s );
	remove_screen_from_list( s );
}


