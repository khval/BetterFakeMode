

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <exec/emulation.h>
#include <exec/ports.h>

#include "engine.h"
#include "EngineTimer.h"
#include "helper/screen.h"

extern unsigned char *bits2bytes[256*8];
extern APTR video_mutex;
extern bool quit;
extern BPTR output;

struct amiga_rgb
{
	uint32 r;
	uint32 g;
	uint32 b;
};

uint32 argb[256];

void comp_window_update( int sw, int sh, struct BitMap *bitmap, struct Window *win);

void update_argb_lookup( struct ColorMap *cm )
{
	int c;
	uint32 r,g,b;
	int colors = cm -> Count;
	ULONG d[3];

	for (c=0;c<colors;c++)
	{
		GetRGB32( cm, c, 1, d );

		r = *((u8 *) (d + 0))  << 16;
		g = *((u8 *) (d + 1)) << 8;
		b = *((u8 *) (d + 2)) ;
		argb[ c ] = 0xFF000000 | r | g | b;
	}
}


void draw_bits(struct RastPort *rp, unsigned char *b, int x,int  y )
{
	WritePixelColor(rp,  x++,  y,  argb[ *b++ ] );
	WritePixelColor(rp,  x++,  y,  argb[ *b++ ] );
	WritePixelColor(rp,  x++,  y,  argb[ *b++ ] );
	WritePixelColor(rp,  x++,  y,  argb[ *b++ ] );

	WritePixelColor(rp,  x++,  y,  argb[ *b++ ] );
	WritePixelColor(rp,  x++,  y,  argb[ *b++ ] );
	WritePixelColor(rp,  x++,  y,  argb[ *b++ ] );
	WritePixelColor(rp,  x++,  y,  argb[ *b++ ] );

}


void draw_bits_argb( unsigned char *dest_ptr,unsigned int dest_bpr, unsigned char *b, int x,int  y )
{
	uint32 *d_argb = (uint32 *)	(dest_ptr + dest_bpr * y + (x*4));

	*d_argb++ = argb[ *b++ ]; // 0
	*d_argb++ = argb[ *b++ ]; // 1
	*d_argb++ = argb[ *b++ ]; // 2
	*d_argb++ = argb[ *b++ ]; // 3
	*d_argb++ = argb[ *b++ ]; // 4
	*d_argb++ = argb[ *b++ ]; // 5
	*d_argb++ = argb[ *b++ ]; // 6
	*d_argb++ = argb[ *b++ ]; // 7
}

uint64 draw_1p(unsigned char *ptr, int SizeOfPlane)
{
	return *((uint64 *) bits2bytes[*ptr]);
}

uint64 draw_2p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes[*ptr]);
	return data | *((uint64 *) bits2bytes[256+*(ptr+SizeOfPlane)]);
}

uint64 draw_3p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data =	*((uint64 *) bits2bytes[*ptr]);			ptr+= SizeOfPlane;
	data |=		*((uint64 *) bits2bytes[1*256+*ptr]);	ptr+= SizeOfPlane;
	return data |	*((uint64 *) bits2bytes[2*256+*ptr]);
}

uint64 draw_4p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[1*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[2*256+*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes[3*256+*(ptr)]);
}

uint64 draw_5p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[1*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[2*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[3*256+*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes[4*256+*(ptr)]);
}

uint64 draw_6p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[1*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[2*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[3*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[4*256+*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes[5*256+*(ptr)]);
}

uint64 draw_7p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[1*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[2*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[3*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[4*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[5*256+*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes[6*256+*(ptr)]);
}

uint64 draw_8p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[1*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[2*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[3*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[4*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[5*256+*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes[6*256+*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes[7*256+*(ptr)]);
};

// basicly unrolled routines.

void *planar_routines[]  =
{
	NULL,
	draw_1p,
	draw_2p,
	draw_3p,
	draw_4p,
	draw_5p,
	draw_6p,
	draw_7p,
	draw_8p
};


void draw_screen( struct Window *win, struct BitMap *bm, struct BitMap *dest_bm )
{
//	struct RastPort *rp = win -> RPort;
	int SizeOfPlane;
	int x,y;
	int bx,bpr;
	unsigned char *yptr;
	uint64 data = 0;
	uint max_height;

	ULONG dest_format;
	unsigned char *dest_ptr;
	unsigned int dest_bpr;
	APTR lock;

	bpr = bm -> BytesPerRow;
	SizeOfPlane = bm -> BytesPerRow * bm -> Rows;

	uint64 (*planar_routine) (unsigned char *ptr, int SizeOfPlane) = NULL;

	planar_routine = planar_routines[ bm -> Depth ];

	lock = LockBitMapTags( dest_bm,
			LBM_PixelFormat, &dest_format,
			LBM_BytesPerRow, &dest_bpr,
			LBM_BaseAddress, &dest_ptr,
			TAG_END	 );

	max_height = bm -> Rows > 480 ? 480 : bm -> Rows;	// must limit....

	for (y=0;y<max_height; y++)
	{
		x = 0;
		yptr = bm -> Planes[0] + bpr*y;

		for (bx=0;bx<bpr;bx++)
		{
			data = planar_routine( yptr + bx, SizeOfPlane );
			draw_bits_argb( dest_ptr,dest_bpr, (unsigned char *) &data ,  x,  y );
			x+=8;
		}
	}

	UnlockBitMap( lock );
}




void dump_screen()
{
	struct TimerContext tc;
	struct Screen *src;
	ULONG win_mask = 0;
	struct BitMap *dest_bitmap;

	bzero( &tc , sizeof(struct TimerContext) );

	struct Window *win = OpenWindowTags( NULL, 
			WA_InnerWidth, 640,
			WA_InnerHeight, 480,
			WA_DragBar, TRUE,
			TAG_END);

	if (!win) return ;

	dest_bitmap =AllocBitMap( 640, 480, 32, BMF_DISPLAYABLE, win ->RPort -> BitMap);

	if (!dest_bitmap) return ;

	if (open_timer_context( &tc)== false)
	{
		close_timer_context( &tc );
		CloseWindow( win );
		return ;
	}

	win_mask = win -> UserPort ? 1 << win -> UserPort ->mp_SigBit : 0;

	do
	{
		ULONG sig = Wait( win_mask | tc.timer_mask | SIGBREAKF_CTRL_C);
		LONG src_width, src_height;

		if (sig & SIGBREAKF_CTRL_C)	break;

		if (sig & tc.timer_mask)
		{
			src_width = -1;
			src_height = -1;

			MutexObtain(video_mutex);
			src = first_fake_screen();
			if (src)
			{
				update_argb_lookup( src -> ViewPort.ColorMap );
				draw_screen( win,  src -> RastPort.BitMap, dest_bitmap );
				src_width = src -> Width;
				src_height = src -> Height;

			}
			MutexRelease(video_mutex);

			if (src_width>-1) comp_window_update( src_width, src_height, dest_bitmap, win);

			reset_timer( tc.timer_io );
		}

	} while (!quit);

	close_timer_context( &tc );
	CloseWindow( win );
	FreeBitMap( dest_bitmap );
}


struct XYSTW_Vertex3D { 
float x, y; 
float s, t, w; 
}; 

typedef struct CompositeHookData_s {
	struct BitMap *srcBitMap; // The source bitmap
	int32 srcWidth, srcHeight; // The source dimensions
	int32 offsetX, offsetY; // The offsets to the destination area relative to the window's origin
	int32 scaleX, scaleY; // The scale factors
	uint32 retCode; // The return code from CompositeTags()
} CompositeHookData;

/*
// New code I can't get to work yet....

static ULONG compositeHookFunc(
			struct Hook *hook, 
			struct RastPort *rastPort, 
			struct BackFillMessage *msg)
 {

	struct BitMap *bm = (struct BitMap *) hook->h_Data;

	int src_width = 640;
	int src_height = 480;
	int DestWidth = msg->Bounds.MaxX - msg->Bounds.MinX;
	int DestHeight = msg->Bounds.MaxY - msg->Bounds.MinY;
	int offsetX = 0;
	int offsetY = 0;

	CompositeTags(
		COMPOSITE_Src, bm, rastPort->BitMap,

		COMPTAG_SrcWidth,   src_width,
		COMPTAG_SrcHeight,  src_height,
		COMPTAG_ScaleX, 	COMP_FLOAT_TO_FIX( (float) DestWidth /  (float) src_width),
		COMPTAG_ScaleY, 	COMP_FLOAT_TO_FIX( (float) DestHeight / (float) src_height),
		COMPTAG_OffsetX,    msg->Bounds.MinX - (msg->OffsetX - msg->Bounds.MinX),
		COMPTAG_OffsetY,    msg->Bounds.MinY - (msg->OffsetY - msg->Bounds.MinY),
		COMPTAG_DestX,      msg->Bounds.MinX,
		COMPTAG_DestY,      msg->Bounds.MinY,
		COMPTAG_DestWidth, DestWidth,
		COMPTAG_DestHeight, DestHeight ,
		COMPTAG_Flags,      COMPFLAG_SrcFilter | COMPFLAG_IgnoreDestAlpha | COMPFLAG_HardwareOnly,
		TAG_END);

	return 0;
}
*/


static ULONG compositeHookFunc(
			struct Hook *hook, 
			struct RastPort *rastPort, 
			struct BackFillMessage *msg)
 {

	CompositeHookData *hookData = (CompositeHookData*)hook->h_Data;

	hookData->retCode = CompositeTags(
		COMPOSITE_Src, 
			hookData->srcBitMap, 
			rastPort->BitMap,
		COMPTAG_SrcWidth,   hookData->srcWidth,
		COMPTAG_SrcHeight,  hookData->srcHeight,
		COMPTAG_ScaleX, 	hookData->scaleX,
		COMPTAG_ScaleY, 	hookData->scaleY,
		COMPTAG_OffsetX,    msg->Bounds.MinX - (msg->OffsetX - hookData->offsetX),
		COMPTAG_OffsetY,    msg->Bounds.MinY - (msg->OffsetY - hookData->offsetY),
		COMPTAG_DestX,      msg->Bounds.MinX,
		COMPTAG_DestY,      msg->Bounds.MinY,
		COMPTAG_DestWidth,  msg->Bounds.MaxX - msg->Bounds.MinX + 1,
		COMPTAG_DestHeight, msg->Bounds.MaxY - msg->Bounds.MinY + 1,
		COMPTAG_Flags,      COMPFLAG_SrcFilter | COMPFLAG_IgnoreDestAlpha | COMPFLAG_HardwareOnly,
		TAG_END);

	return 0;
}


void comp_window_update( int src_width, int src_height, struct BitMap *bitmap, struct Window *win)
{
	struct Hook hook;
	static CompositeHookData hookData;
	struct Rectangle rect;
	register struct RastPort *RPort = win->RPort;

 	rect.MinX = win->BorderLeft;
 	rect.MinY = win->BorderTop;
 	rect.MaxX = win->Width - win->BorderRight ;
 	rect.MaxY = win->Height - win->BorderBottom ;

	hook.h_Entry = (HOOKFUNC) compositeHookFunc;
	hook.h_Data = &hookData;

	hookData.srcBitMap = bitmap;
	hookData.srcWidth = src_width;
	hookData.srcHeight = src_height > 480 ? 480 : src_height;
	hookData.offsetX = win->BorderLeft;
	hookData.offsetY = win->BorderTop;
	hookData.retCode = COMPERR_Success;

 	float destWidth = rect.MaxX - rect.MinX + 1;
 	float destHeight = rect.MaxY - rect.MinY + 1;

 	float scaleX = (destWidth + 0.5f) / hookData.srcWidth;
 	float scaleY = (destHeight + 0.5f) / hookData.srcHeight;

	hookData.scaleX = COMP_FLOAT_TO_FIX(scaleX);
	hookData.scaleY = COMP_FLOAT_TO_FIX(scaleY);

	LockLayer(0, RPort->Layer);
	DoHookClipRects(&hook, win->RPort, &rect);
	UnlockLayer( RPort->Layer);
}



