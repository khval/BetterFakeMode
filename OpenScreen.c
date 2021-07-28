
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/diskfont.h>
#include <exec/emulation.h>
#include <graphics/modeid.h>

#include "common.h"
#include "modeid.h"
#include "helper/screen.h"


void initColors( struct ViewPort *vp)
{
	struct ColorMap *cm = vp -> ColorMap;

	 SetRGB32( vp, 0, 0x55555555,0x55555555,0x55555555 );
	 SetRGB32( vp, 1, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF );

	if (cm -> Count>2)	// 4 colors or up.
	{
		 SetRGB32( vp, 2, 0x00000000,0x00000000,0x00000000 );
		 SetRGB32( vp, 3, 0xFFFFFFFF,0x00000000,0x00000000 );
	}

	if (cm -> Count>4) // 8 colors or up.
	{ 
		 SetRGB32( vp, 4, 0x00000000,0xFFFFFFFF,0x00000000 );
		 SetRGB32( vp, 5, 0x00000000,0x00000000,0xFFFFFFFF );
		 SetRGB32( vp, 6, 0x00000000,0x00000000,0x00000000 );
		 SetRGB32( vp, 7, 0x00000000,0x00000000,0x00000000 );
	}
}

void fake_initRasInfo( struct RasInfo *ri, struct BitMap *bm )
{
	ri -> BitMap = bm;
}

void fake_initViewPort( struct Screen *s , int depth )
{
	struct ViewPort *vp = &s-> ViewPort;

	if (vp -> DWidth == 0) vp -> DWidth = s -> Width;
	if (vp -> DHeight == 0) vp -> DHeight = s -> Height;

	vp -> ColorMap = GetColorMap( 1L << depth);
	if (vp -> ColorMap)
	{
		vp -> ColorMap -> cm_vp = vp;
		initColors( vp );
	}

	vp -> RasInfo = new_struct( RasInfo );
	if (vp -> RasInfo) fake_initRasInfo( vp -> RasInfo, s -> RastPort.BitMap );
}
#if use_fake_bitmap == 1

#warning using fake bitmap

struct BitMap *_new_fake_bitmap(int Width,int Height, int Depth)
{
	int d;
	struct BitMap *bm = new_struct( BitMap);
	int sizeOfPlane;
	unsigned char *data;

	if (bm == NULL) return NULL;

	InitBitMap( bm, Depth, Width, Height );
	bm -> pad = 0xFA8E;	// Fake BitMap!!

	sizeOfPlane = bm -> BytesPerRow *  bm -> Rows;

	data = AllocVecTags( sizeOfPlane * Depth , AVT_Type, MEMF_SHARED, AVT_ClearWithValue, 0, TAG_END); 

	for (d=0;d<Depth;d++)
	{
		bm -> Planes[d] =  data + sizeOfPlane*d ;
	}

	return bm;
}
#endif

void _init_fake_screen(struct Screen *s,int Depth )
{
	s -> Width = s -> Width & 15 ? (s -> Width + 16) & 0xFFFE : s -> Width ;	// Round up closes 16 pixels.

#if use_fake_bitmap == 1

	if (s -> RastPort.BitMap == NULL) 
	{
		s -> RastPort.BitMap = _new_fake_bitmap( s-> Width, s -> Height, Depth );
	}

#else

	s -> RastPort.BitMap = AllocBitMapTags(	s-> Width, s -> Height, Depth,
				BMATags_PixelFormat, PIXF_NONE,
				BMATags_UserPrivate, TRUE,
				TAG_END	 );

#endif

	if (s -> Font == NULL)	// no font set.
	{
		s -> Font = default_font;
		SetFont( &s -> RastPort , s -> Font );
	}

	s -> BitMap  = * (s -> RastPort.BitMap);

	FPrintf( output, "BytesPerRow: %ld\n", s ->RastPort.BitMap -> BytesPerRow );

	 fake_initViewPort( s, Depth);

	// need to fill in the struct as best as I can.
}

void add_LayerInfo( int screenIndex, struct Screen *s )
{
	allocatedScreen[screenIndex] = true;
	LayerInfos[screenIndex] = NewLayerInfo();

	if (LayerInfos[screenIndex])
	{
		memcpy( &s -> LayerInfo, LayerInfos[screenIndex] , sizeof(struct Layer_Info) );
	}
}

void init_screen_from_newScreen( const struct NewScreen * newScreen, struct Screen *s, ULONG *depth )
{
	s -> Width = newScreen -> Width;
	s -> Height = newScreen -> Height;
	*depth = newScreen -> Depth;

	if (newScreen -> CustomBitMap)
	{
		s -> RastPort.BitMap = newScreen -> CustomBitMap;
	}

	if (newScreen -> DefaultTitle)
	{
		s -> Title = strdup(newScreen -> DefaultTitle);
	}

	if (newScreen -> Font)
	{
		s -> Font = OpenDiskFont( newScreen -> Font );
		if ( ! s -> Font ) s -> Font = default_font;
		SetFont( &s -> RastPort , s -> Font );
	}

/*
		newScreen -> Type,
		newScreen -> ViewModes );
*/
}



void update_screen_from_taglist(const struct TagItem * tagList, struct Screen *s, ULONG *depth)
{
	int s_w, s_h, s_d, s_monitor;

	const struct TagItem * tag;

	for (tag = tagList; tag -> ti_Tag != TAG_DONE; tag++)
	{
		switch (tag -> ti_Tag)
		{
			case SA_Width:
				s_w = tag -> ti_Data;
				s -> Width = s_w;
				break;

			case SA_Height:
				s_h = tag -> ti_Data;
				s -> Height = s_w;
				break;

			case SA_Depth:
				s_d = tag -> ti_Data;
				*depth = s_d;
				break;

			case SA_DisplayID:
				s -> ViewPort.Modes = tag -> ti_Data;
				s_monitor = tag -> ti_Data & MONITOR_ID_MASK;
				break;
		}
	}

	{
		struct modeT *mode;

		mode = bestMode( s_monitor,  s_w,  s_h);
		if (mode)
		{
			s -> ViewPort.DWidth = mode -> w;
			s -> ViewPort.DHeight = mode -> h;

			FPrintf( output, "ViewPort %ld,%ld\n", mode -> w, mode -> h);
		}
	}

}

struct Screen * _new_fake_OpenScreenTagList( const struct NewScreen * newScreen, const struct TagItem * tagList)
{
	ULONG depth = 0;
	int screenIndex;
	struct Screen *s;

	screenIndex = alloc_screen_in_list();
	s = screenIndex >-1 ? screens+screenIndex : NULL;

	if (s)
	{
		InitVPort( &s -> ViewPort );
		InitRastPort( &s -> RastPort );

		s -> ViewPort.DHeight = 0;
		s -> ViewPort.DWidth = 0;

		s -> FirstWindow = NULL;
		if (s -> Title) s -> Title = strdup("Hello World");

		add_LayerInfo( screenIndex, s );

		if (newScreen) init_screen_from_newScreen( newScreen, s, &depth );
		if (tagList)
		{
			dump_tags( tagList , screen_info_tags);
			update_screen_from_taglist(tagList, s, &depth);
		}

		if (s -> Width == 0) s -> Width = 320;
		if (s -> Height == 0) s -> Height = 200;

		_init_fake_screen(s,depth);

		return s;
	}
	return NULL;
}

