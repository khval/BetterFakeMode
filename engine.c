

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

void draw_screen( struct Window *win, struct BitMap *bm )
{
	struct RastPort *rp = win -> RPort;
	int SizeOfPlane;
	int x,y,p,d;
	int bx,bpr;
	int yy;

	unsigned char *ptr;
	uint64 *at;
	uint64 data = 0;

	d = bm -> Depth;

	bpr = bm -> BytesPerRow;
	SizeOfPlane = bm -> BytesPerRow * bm -> Rows;

	for (y=0;y<bm -> Rows; y++)
	{
		yy = y +win -> BorderTop;
		x = win -> BorderLeft;
		for (bx=0;bx<bpr;bx++)
		{
			data = 0;

			ptr = bm -> Planes[0] + bpr*y + bx;

			for (p=0;p<d;p++)
			{
				at = (uint64 *) bits2bytes[256*p+*ptr];
				data |= *at;
				ptr += SizeOfPlane;			
			}

			draw_bits( rp, (unsigned char *) &data ,  x,  yy );

			x+=8;
		}
	}
}

void dump_screen()
{
	struct TimerContext tc;
	struct Screen *src;
	ULONG win_mask = 0;

	bzero( &tc , sizeof(struct TimerContext) );

	struct Window *win = OpenWindowTags( NULL, 
			WA_InnerWidth, 640,
			WA_InnerHeight, 480,
			WA_DragBar, TRUE,
			TAG_END);


	if (!win) return ;

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
				draw_screen( win, src -> RastPort.BitMap );
			}
			MutexRelease(video_mutex);

			reset_timer( tc.timer_io );
		}

	} while (!quit);

	close_timer_context( &tc );
	CloseWindow( win );
}


