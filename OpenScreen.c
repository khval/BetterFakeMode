
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"

/*
struct Screen
{
    struct Screen *NextScreen;  		// we are not going to link into the OS.
    struct Window *FirstWindow; 	// we are not going to link into the OS.
    WORD LeftEdge, TopEdge;   		// ignore, i think.
    WORD Width, Height;   
    int16 MouseY;              
    int16 MouseX;
    UWORD Flags;      
    STRPTR Title;         
    STRPTR DefaultTitle;
    BYTE BarHeight, BarVBorder, BarHBorder, MenuVBorder, MenuHBorder;
    BYTE WBorTop, WBorLeft, WBorRight, WBorBottom;
    struct TextAttr *Font;  
    struct ViewPort ViewPort;  
    struct RastPort RastPort;  
    struct BitMap BitMap;    
    struct Layer_Info LayerInfo;
    struct Gadget *FirstGadget;
    UBYTE DetailPen, BlockPen;   
    UWORD SaveColor0;
    struct Layer *BarLayer;
    UBYTE *ExtData;
    UBYTE *UserData;    
};

struct ViewPort
{
    struct ViewPort *Next;
    struct ColorMap *ColorMap; [****]
    struct CopList  *DspIns;
    struct CopList  *SprIns;
    struct CopList  *ClrIns;
    struct UCopList *UCopIns;
    WORD             DWidth;
    WORD             DHeight;
    WORD             DxOffset;
    WORD             DyOffset;
    UWORD            Modes;
    UBYTE            SpritePriorities;
    UBYTE            ExtendedModes;
    struct RasInfo  *RasInfo;
};

struct BitMap
{
    uint16   BytesPerRow;
    uint16   Rows;
    uint8    Flags;
    uint8    Depth;
    uint16   pad;
    PLANEPTR Planes[8];
};

struct ColorMap
{
    UBYTE                 Flags;
    UBYTE                 Type;
    UWORD                 Count; [***]
    APTR                  ColorTable; [***]
    struct ViewPortExtra *cm_vpe;
    APTR                  LowColorBits;
    UBYTE                 TransparencyPlane;
    UBYTE                 SpriteResolution;
    UBYTE                 SpriteResDefault; 
    UBYTE                 AuxFlags;
    struct ViewPort      *cm_vp;
    APTR                  NormalDisplayInfo;
    APTR                  CoerceDisplayInfo;
    struct TagItem       *cm_batch_items;
    ULONG                 VPModeID;
    struct PaletteExtra  *PalExtra;
    UWORD                 SpriteBase_Even;
    UWORD                 SpriteBase_Odd;
    UWORD                 Bp_0_base;
    UWORD                 Bp_1_base;
};

*/

void fake_initColorMap( struct ViewPort *vp, int depth)
{
	struct ColorMap *cm = vp -> ColorMap;

	cm -> Count = 1L << depth;
	cm -> ColorTable = AllocVecTags( sizeof(uint32) * 4  * cm -> Count, 
			AVT_Type, MEMF_SHARED, AVT_ClearWithValue, 0, TAG_END); 
	cm -> cm_vp = vp;
}

void fake_initViewPort(struct ViewPort *vp, int depth )
{
	vp -> ColorMap =  new_struct( ColorMap );
	if (vp -> ColorMap) fake_initColorMap( vp, depth );
}

struct BitMap *_new_fake_bitmap(int Width,int Height, int Depth)
{
	int d;
	struct BitMap *bm = new_struct( BitMap);

	if (bm == NULL) return NULL;

	InitBitMap( bm, Depth, Width, Height );

/*
	bm -> BytesPerRow = Width / 8;
	bm -> Rows = Height;
	bm -> Depth = Depth;
*/

	for (d=0;d<Depth;d++)
	{
		bm -> Planes[d] =  AllocVecTags( bm -> BytesPerRow *  bm -> Rows, 
			AVT_Type, MEMF_SHARED, AVT_ClearWithValue, 0, TAG_END); 
	}

	return bm;
}

struct Screen *_new_fake_screen(int Width, int Height, int Depth)
{
	struct Screen *s;
	struct BitMap *bm;

	s = (struct Screen *) AllocVecTags( sizeof(struct Screen), 
		AVT_Type, MEMF_SHARED,
		AVT_ClearWithValue, 0,
		TAG_END); 

	if (s)
	{
		s -> Width = Width & 15 ? (Width + 16) & 0xFFFE : Width ;	// Round up closes 16 pixels.
		s -> Height = Height;

		InitVPort( &s-> ViewPort );
		InitRastPort( & s -> RastPort );

#if use_fake_bitmap == 1
		s -> RastPort.BitMap = _new_fake_bitmap( s-> Width, s -> Height, Depth );
#else
		s -> RastPort.BitMap = AllocBitMapTags(	s-> Width, s -> Height, Depth,
				BMATags_PixelFormat, PIXF_NONE,
				BMATags_UserPrivate, TRUE,
				TAG_END	 );
#endif

		FPrintf( output, "%ld\n", s ->RastPort.BitMap -> BytesPerRow );

		 fake_initViewPort( &s-> ViewPort, Depth );

		// need to fill in the struct as best as I can.
		return s;
	}

	return NULL;
}

