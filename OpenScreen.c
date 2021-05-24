
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


void fake_initColorMap( struct ViewPort *vp, int depth)
{
	struct ColorMap *cm = vp -> ColorMap;

	cm -> Count = 1L << depth;
	cm -> ColorTable = AllocVecTags( sizeof(uint32) * 3  * cm -> Count, 
			AVT_Type, MEMF_SHARED, AVT_ClearWithValue, 0, TAG_END); 
	cm -> cm_vp = vp;

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

	InitVPort( vp );

	vp -> DWidth = s -> Width;
	vp -> DHeight = s -> Height;

	vp -> ColorMap =  new_struct( ColorMap );
	if (vp -> ColorMap) fake_initColorMap( vp, depth );

	vp -> RasInfo = new_struct( RasInfo );
	if (vp -> RasInfo) fake_initRasInfo( vp -> RasInfo, s -> RastPort.BitMap );
}

struct BitMap *_new_fake_bitmap(int Width,int Height, int Depth)
{
	int d;
	struct BitMap *bm = new_struct( BitMap);
	int sizeOfPlane;
	unsigned char *data;

	if (bm == NULL) return NULL;

	InitBitMap( bm, Depth, Width, Height );

	sizeOfPlane = bm -> BytesPerRow *  bm -> Rows;

	data = AllocVecTags( sizeOfPlane * Depth , AVT_Type, MEMF_SHARED, AVT_ClearWithValue, 0, TAG_END); 

	for (d=0;d<Depth;d++)
	{
		bm -> Planes[d] =  data + sizeOfPlane*d ;
	}

	return bm;
}

void _init_fake_screen(struct Screen *s,int Depth)
{
		s -> Width = s -> Width & 15 ? (s -> Width + 16) & 0xFFFE : s -> Width ;	// Round up closes 16 pixels.

		InitRastPort( & s -> RastPort );

#if use_fake_bitmap == 1
		s -> RastPort.BitMap = _new_fake_bitmap( s-> Width, s -> Height, Depth );
#else
		s -> RastPort.BitMap = AllocBitMapTags(	s-> Width, s -> Height, Depth,
				BMATags_PixelFormat, PIXF_NONE,
				BMATags_UserPrivate, TRUE,
				TAG_END	 );
#endif

		s -> Font = default_font;

		SetFont( &s -> RastPort , default_font );

		s -> BitMap  = * (s -> RastPort.BitMap);

		FPrintf( output, "%ld\n", s ->RastPort.BitMap -> BytesPerRow );

		 fake_initViewPort( s, Depth );

		// need to fill in the struct as best as I can.
}

void add_LayerInfo( int i, struct Screen *s )
{
	allocatedScreen[i] = true;
	LayerInfos[i] = NewLayerInfo();

	if (LayerInfos[i])
	{
		memcpy( &s -> LayerInfo, LayerInfos[i] , sizeof(struct Layer_Info) );
	}
}

void init_screen_from_newScreen( const struct NewScreen * newScreen, struct Screen *s, int *depth )
{
	s -> Width = newScreen -> Width;
	s -> Height = newScreen -> Height;
	*depth = newScreen -> Depth;
}

void update_screen_from_taglist(const struct TagItem * tagList, struct Screen *s, int *depth)
{
	const struct TagItem * tag;

	for (tag = tagList; tag -> ti_Tag != TAG_DONE; tag++)
	{
		switch (tag -> ti_Tag)
		{
			case SA_Width:
				s -> Width = tag -> ti_Data;
				break;

			case SA_Height:
				s -> Height = tag -> ti_Data;
				break;

			case SA_Depth:
				*depth = tag -> ti_Data;
				break;
		}
	}
}

struct Screen * _new_fake_OpenScreenTagList( const struct NewScreen * newScreen, const struct TagItem * tagList)
{
	int depth;
	int i;
	struct Screen *s;

	i = alloc_screen_in_list();
	s = i>-1 ? screens+i : NULL;

	if (s)
	{
		s -> FirstWindow = NULL;

		s -> Title = strdup("Hello World");

		add_LayerInfo( i, s );

		if (newScreen) init_screen_from_newScreen( newScreen, s, &depth );
		if (tagList)
		{
			update_screen_from_taglist(tagList, s, &depth);
		}

		_init_fake_screen(s,depth);

		return s;
	}
	return NULL;
}

