

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

struct amiga_rgb
{
	uint32 r;
	uint32 g;
	uint32 b;
};

uint32 argb[256];

void comp_window_update( struct BitMap *bitmap, struct Window *win);

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

	for (y=0;y<bm -> Rows; y++)
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

		if (sig & SIGBREAKF_CTRL_C)	break;

		if (sig & tc.timer_mask)
		{
			MutexObtain(video_mutex);
			src = first_fake_screen();
			if (src)
			{
				update_argb_lookup( src -> ViewPort.ColorMap );
				draw_screen( win,  src -> RastPort.BitMap, dest_bitmap );
			}
			MutexRelease(video_mutex);

			comp_window_update( dest_bitmap, win);

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

void comp_window_update( struct BitMap *bitmap, struct Window *win)
{
	#define STEP(a,xx,yy,ss,tt,ww)   P[a].x= xx; P[a].y= yy; P[a].s= ss; P[a].t= tt; P[a].w= ww;  

	int width,height;

	int ww,wh;
	int error;
	float sx;
	float sy;

	float wx;
	float wy;

	struct XYSTW_Vertex3D P[6];

	width=640;
	height=480;

	wx = win->BorderLeft + win -> LeftEdge;
	wy = win->BorderTop + win -> TopEdge;

	ww = win->Width - win->BorderLeft - win->BorderRight;
	wh = win->Height -  win->BorderTop - win->BorderBottom;

	STEP(0, wx, wy ,0 ,0 ,1);
	STEP(1, wx+ww,wy,width,0,1);
	STEP(2, wx+ww,wy+wh,width,height,1);

	STEP(3, wx,wy, 0,0,1);
	STEP(4, wx+ww,wy+wh,width,height,1);
	STEP(5, wx, wy+wh ,0 ,height ,1);

	LockLayer( 0 , win -> RPort -> Layer);

	error = CompositeTags(COMPOSITE_Src, 
			bitmap, win->RPort -> BitMap,

			COMPTAG_VertexArray, P, 
			COMPTAG_VertexFormat,COMPVF_STW0_Present,
		    	COMPTAG_NumTriangles,2,

			COMPTAG_ScaleX, (uint32) ( (float) 0x0010000 * sx ),
			COMPTAG_ScaleY, (uint32) ( (float) 0x0010000 * sy ),

			COMPTAG_SrcAlpha, (uint32) (0x0010000 ),
			COMPTAG_Flags, COMPFLAG_SrcAlphaOverride | COMPFLAG_HardwareOnly | COMPFLAG_SrcFilter ,
			TAG_DONE);

	UnlockLayer( win -> RPort -> Layer );

}

