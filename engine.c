

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

#include "common.h"
#include "engine.h"
#include "EngineTimer.h"
#include "helper/screen.h"

extern unsigned char *bits2bytes[256*8];

extern unsigned char **bits2bytes0 ;
extern unsigned char **bits2bytes1 ;
extern unsigned char **bits2bytes2 ;
extern unsigned char **bits2bytes3 ;
extern unsigned char **bits2bytes4 ;
extern unsigned char **bits2bytes5 ;
extern unsigned char **bits2bytes6 ;
extern unsigned char **bits2bytes7 ;

extern APTR video_mutex;
extern bool quit;
extern BPTR output;

extern ULONG host_sig;
extern struct Task *host_task;

extern struct TextFont *default_font;

struct MsgPort *reply_port = NULL;

struct amiga_rgb
{
	uint32 r;
	uint32 g;
	uint32 b;
};

union amiga_argb
{
	uint32 argb;
	struct
	{
		unsigned char a;
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};
};

union amiga_argb palette[256];

void comp_window_update( struct Screen *src, struct BitMap *bitmap, struct Window *win);

void update_argb_lookup( struct ColorMap *cm )
{
	int c;
	int colors = cm -> Count;
	ULONG d[3];

	for (c=0;c<colors;c++)
	{
		GetRGB32( cm, c, 1, d );

		d[0] >>= 24;
		d[1] >>= 24;
		d[2] >>= 24;

		palette[ c ].a = 0xFF;
		palette[ c ].r = d[0];
		palette[ c ].g = d[1];
		palette[ c ].b = d[2];
	}
}

void dump_colors( struct ColorMap *cm )
{
	int c;
	int colors = cm -> Count;
	ULONG d[3];

	for (c=0;c<colors;c++)
	{
		GetRGB32( cm, c, 1, d );
		FPrintf( output,"%ld: %08lx,%08lx,%08lx\n", c, d[0] , d[1] , d[2]);
	}

}

/*
void draw_bits(struct RastPort *rp, unsigned char *b, int x,int  y )
{
	WritePixelColor(rp,  x++,  y,  palette[ *b++ ].argb );
	WritePixelColor(rp,  x++,  y,  palette[ *b++ ].argb );
	WritePixelColor(rp,  x++,  y,  palette[ *b++ ].argb );
	WritePixelColor(rp,  x++,  y,  palette[ *b++ ].argb );

	WritePixelColor(rp,  x++,  y,  palette[ *b++ ].argb );
	WritePixelColor(rp,  x++,  y,  palette[ *b++ ].argb );
	WritePixelColor(rp,  x++,  y,  palette[ *b++ ].argb );
	WritePixelColor(rp,  x,  y,  palette[ *b ].argb );

}
*/

union amiga_argb ham;

inline uint32 ham6( unsigned char color )
{
	switch (color & 0x30)
	{
		case 0x00:
				ham.argb = palette[color].argb ;
				break;
		case 0x10:
				ham.b = ((color & 0xF) * 0x11);
				break;
		case 0x20:
				ham.r = ((color & 0xF) * 0x11);
				break;
		case 0x30:
				ham.g = ((color & 0xF) * 0x11);
				break;
	}

	return ham.argb;
}

inline uint32 ham8( unsigned char color )
{
	switch (color & 0x3)
	{
		case 0x0:
				ham.argb = palette[color].argb;
				break;
		case 0x1:
				ham.b = color & 0xFC;
				break;
		case 0x2:
				ham.g = (color & 0xFC);
				break;
		case 0x3:
				ham.r = (color & 0xFC);
				break;
	}

	return ham.argb;
}

void draw_bits_argb( unsigned char *dest_ptr,unsigned int dest_bpr, unsigned char *b, int x,int  y )
{
	uint32 *d_argb = (uint32 *)	(dest_ptr + dest_bpr * y + (x*4));

	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb = palette[ *b ].argb ;
}

void draw_bits_argb_ham6( unsigned char *dest_ptr,unsigned int dest_bpr, unsigned char *b, int x,int  y )
{
	uint32 *d_argb = (uint32 *)	(dest_ptr + dest_bpr * y + (x*4));

	*d_argb++ = ham6( *b++ ); // 0
	*d_argb++ = ham6( *b++ ); // 1
	*d_argb++ = ham6( *b++ ); // 2
	*d_argb++ = ham6( *b++ ); // 3
	*d_argb++ = ham6( *b++ ); // 4
	*d_argb++ = ham6( *b++ ); // 5
	*d_argb++ = ham6( *b++ ); // 6
	*d_argb = ham6( *b ); // 7
}

void draw_bits_argb_ham8( unsigned char *dest_ptr,unsigned int dest_bpr, unsigned char *b, int x,int  y )
{
	uint32 *d_argb = (uint32 *)	(dest_ptr + dest_bpr * y + (x*4));

	*d_argb++ = ham8( *b++ ); // 0
	*d_argb++ = ham8( *b++ ); // 1
	*d_argb++ = ham8( *b++ ); // 2
	*d_argb++ = ham8( *b++ ); // 3
	*d_argb++ = ham8( *b++ ); // 4
	*d_argb++ = ham8( *b++ ); // 5
	*d_argb++ = ham8( *b++ ); // 6
	*d_argb = ham8( *b ); // 7
}

uint64 draw_1p(unsigned char *ptr, int SizeOfPlane)
{
	return *((uint64 *) bits2bytes[*ptr]);
}

uint64 draw_2p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes0[*ptr]);
	return data | *((uint64 *) bits2bytes1[*(ptr+SizeOfPlane)]);
}

uint64 draw_3p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data =	*((uint64 *) bits2bytes0[*ptr]);	ptr+= SizeOfPlane;
	data |=		*((uint64 *) bits2bytes1[*ptr]);	ptr+= SizeOfPlane;
	return data |	*((uint64 *) bits2bytes2[*ptr]);
}

uint64 draw_4p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes0[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes1[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes2[*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes3[*(ptr)]);
}

uint64 draw_5p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes0[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes1[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes2[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes3[*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes4[*(ptr)]);
}

uint64 draw_6p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes0[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes1[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes2[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes3[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes4[*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes5[*(ptr)]);
}

uint64 draw_7p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes0[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes1[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes2[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes3[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes4[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes5[*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes6[*(ptr)]);
}

uint64 draw_8p(unsigned char *ptr, int SizeOfPlane)
{
	uint64 data = *((uint64 *) bits2bytes0[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes1[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes2[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes3[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes4[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes5[*ptr]); ptr+= SizeOfPlane;
	data |= *((uint64 *) bits2bytes6[*ptr]); ptr+= SizeOfPlane;
	return data | *((uint64 *) bits2bytes7[*(ptr)]);
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


void draw_screen( struct emuIntuitionContext *c) 
{
	struct BitMap *bm = c -> src -> RastPort.BitMap;
	struct BitMap *dest_bm = c -> dest_bitmap;

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

	uint64 (*planar_routine) (unsigned char *ptr, int SizeOfPlane) = NULL;
	void (*draw_bits) ( unsigned char *dest_ptr,unsigned int dest_bpr, unsigned char *b, int x,int  y ) = NULL;

	bpr = bm -> BytesPerRow;
	SizeOfPlane = bm -> BytesPerRow * bm -> Rows;
	planar_routine = planar_routines[ bm -> Depth ];

	switch ( (c -> src -> ViewPort.Modes & 0x800) | bm -> Depth )
	{
		case 0x0806:	draw_bits = draw_bits_argb_ham6;		break;
		case 0x0808:	draw_bits = draw_bits_argb_ham8;		break;
		default:		draw_bits = draw_bits_argb;			break;
	}

	lock = LockBitMapTags( dest_bm,
			LBM_PixelFormat, &dest_format,
			LBM_BytesPerRow, &dest_bpr,
			LBM_BaseAddress, &dest_ptr,
			TAG_END	 );

	max_height = c -> src -> ViewPort.DHeight;
	if (bm -> Rows < max_height) max_height = bm -> Rows;

	for (y=0;y<max_height; y++)
	{
		x = 0;
		yptr = bm -> Planes[0] + bpr*y;

		ham.argb = 0xFF000000;

		for (bx=0;bx<bpr;bx++)
		{
			data = planar_routine( yptr + bx, SizeOfPlane );
			draw_bits( dest_ptr,dest_bpr, (unsigned char *) &data ,  x,  y );
			x+=8;
		}
	}

	UnlockBitMap( lock );
}

void update_fake_window_mouse_xy(struct Screen *src)
{
	struct Window *win;

	for(win = src -> FirstWindow;win;win = win -> NextWindow)
	{
		win -> MouseX = src -> MouseX - win -> LeftEdge;
		win -> MouseY = src -> MouseY - win -> TopEdge;
	}
}

enum 
{
	no_action = 0,
	drag_action,
	size_action,
	close_action,
	wdepth_action
};

LONG mouse_state = 0;
struct Window *active_win=NULL;
LONG clicked_x,clicked_y;

bool ClickButtons( struct Window *win )
{
	ULONG x1,y1;
	LONG mx,my;
	struct Gadget *g;

	mx = win -> MouseX;
	my = win -> MouseY;

	FPrintf( output, "mouse %ld,%ld\n",mx,my);

	for (g = win -> FirstGadget; g ; g = g -> NextGadget )
	{
		x1 = g -> LeftEdge +  g -> Width;
		y1 = g -> TopEdge + g -> Height;
		
		if (
			( mx >= g -> LeftEdge ) && ( mx < x1) &&
			( my >= g -> TopEdge) && ( my < y1)
		)
		{
			switch ( g-> Flags)
			{
				case GTYP_CLOSE: 
					mouse_state = close_action;
					return true;

				case GTYP_WDRAGGING:
					mouse_state = drag_action;
					clicked_x = mx;
					clicked_y = my;
					return true;

				case GTYP_WDEPTH:
					mouse_state = wdepth_action;
					return true;

				case WFLG_SIZEGADGET:
					mouse_state = size_action;
					clicked_x = win -> Width - mx;
					clicked_y = win -> Height -  my;
					return true;

				default:
					return true;
			}
		}		
	}

	return false;
}

UWORD getMinLayerPriority( struct Screen *src )
{
	struct Window *win;
	UWORD min_layer_priority = ~0;

	for(win = src -> FirstWindow;win;win = win -> NextWindow)
	{
		if (win -> WLayer -> priority < min_layer_priority )
		{
			min_layer_priority = win -> WLayer -> priority;
		}
	}

	return min_layer_priority;
}

// return true, if you have clicked on button.

bool WindowClick( struct Screen *src )
{
	ULONG mx,my;
	struct Window *win;
	struct Window *clicked_win;
	UWORD last_clicked_layer_priority = ~0;

	FPrintf( output, "%s\n",__FUNCTION__);

	// update mouse x,y in window, and check what window is clicked.

	clicked_win = NULL;
	for(win = src -> FirstWindow;win;win = win -> NextWindow)
	{
		mx = win -> MouseX = src -> MouseX - win -> LeftEdge;
		my = win -> MouseY = src -> MouseY - win -> TopEdge;

		if (
			(mx>=0) && (mx<win -> Width) &&
			(my>=0) && (my<win -> Height)
		)
		{
			if (win -> WLayer -> priority < last_clicked_layer_priority )
			{
				clicked_win = win;
				last_clicked_layer_priority = win -> WLayer -> priority;
			}
		}
	}

	// if window is clicked, check if any buttons are clicked.

	if (clicked_win)
	{
		no_block_ActivateWindow( clicked_win );
	 	return ClickButtons( clicked_win );
		
	}
	return false;
}

bool window_open(struct Screen *src,struct Window *check_win)
{
	struct Window *win;
	for(win = src -> FirstWindow;win;win = win -> NextWindow)
	{
		win -> MouseX = src -> MouseX - win -> LeftEdge;
		win -> MouseY = src -> MouseY - win -> TopEdge;

		if (win == check_win)
		{
			return true;
		}
	}
	return false;
}

void drag_window(struct Screen *src)
{
	LONG dx,dy;

	FPrintf( output, "%s\n",__FUNCTION__);

	if (window_open(src,active_win))
	{
		dx = active_win -> MouseX - clicked_x;
		dy = active_win -> MouseY - clicked_y;

		if (dx | dy)
		{
			no_block_MoveWindow( active_win, dx, dy  );
		}
	}
}


void wdepth_window(struct Screen *src)
{
	LONG dx,dy;

	FPrintf( output, "%s\n",__FUNCTION__);

	if (window_open(src,active_win))
	{
		ULONG minLayerPriority = getMinLayerPriority( src );
		struct Layer *l = active_win -> RPort -> Layer;

		FPrintf( output, "minLayerPriority: %ld, active_win -> WLayer -> priority: %ld\n",minLayerPriority, active_win -> WLayer -> priority);

		LockLayer(0, active_win -> RPort->Layer);

		if ( minLayerPriority == active_win -> WLayer -> priority)
		{
			BehindLayer( 0, l );
		}
		else UpfrontLayer( 0, l );

		UnlockLayer( active_win -> RPort->Layer);
	}
}

void size_window(struct Screen *src)
{
	int cx,cy;
	int dx,dy;

	FPrintf( output, "%s\n",__FUNCTION__);

	if (window_open(src,active_win))
	{
		cx = clicked_x<0 ? active_win -> Width + clicked_x : clicked_x;
		cy = clicked_y<0 ? active_win -> Height + clicked_y : clicked_y;

		dx = active_win -> MouseX - cx;
		dy = active_win -> MouseY - cy;

		if (dx | dy)
		{
			no_block_SizeWindow( active_win, dx, dy  );
		}
	}
}

void send_copy( struct Window *win,  struct IntuiMessage *source_msg )
{
	if (win -> IDCMPFlags & source_msg -> Class)
	{
		struct IntuiMessage *msg;
		msg = (struct IntuiMessage *) AllocSysObjectTags(ASOT_MESSAGE,
			ASOMSG_Size, sizeof(struct IntuiMessage),
			ASOMSG_ReplyPort, reply_port,
			TAG_DONE);

		if (msg)
		{
			msg -> Class = source_msg -> Class;
			msg -> Code = source_msg -> Code;
			msg -> Qualifier = source_msg -> Qualifier;
			msg -> Seconds = source_msg -> Seconds;
			msg -> Micros = source_msg -> Micros;

			msg -> IDCMPWindow = win;
			msg -> MouseX = win -> MouseX;
			msg -> MouseY = win -> MouseY;

			Forbid();
			PutMsg( win -> UserPort, (struct Message *) msg);
			Permit();
		}
	}
	else
	{
		FPrintf( output, "Window does not have this IDCMP flag\n");
	}
}

void send_mouse_move( struct Window *win,  struct IntuiMessage *source_msg )
{
	if (win -> IDCMPFlags & source_msg -> Class)
	{
		struct IntuiMessage *msg;
		msg = (struct IntuiMessage *) AllocSysObjectTags(ASOT_MESSAGE,
			ASOMSG_Size, sizeof(struct IntuiMessage),
			ASOMSG_ReplyPort, reply_port,
			TAG_DONE);

		if (msg)
		{
			msg -> Class = source_msg -> Class;
			msg -> Code = source_msg -> Code;
			msg -> Qualifier = source_msg -> Qualifier;
			msg -> Seconds = source_msg -> Seconds;
			msg -> Micros = source_msg -> Micros;

			msg -> IDCMPWindow = win;
			msg -> MouseX = win -> MouseX;
			msg -> MouseY = win -> MouseY;


			Forbid();
			PutMsg( win -> UserPort, (struct Message *) msg);
			Permit();
		}
	}
}

void send_INTUITICKS( struct Window *win  )
{
	if (win -> IDCMPFlags & IDCMP_INTUITICKS)
	{
		struct IntuiMessage *msg;
		msg = (struct IntuiMessage *) AllocSysObjectTags(ASOT_MESSAGE,
			ASOMSG_Size, sizeof(struct IntuiMessage),
			ASOMSG_ReplyPort, reply_port,
			TAG_DONE);

		if (msg)
		{
			msg -> Class = IDCMP_INTUITICKS;
			msg -> IDCMPWindow = win;

			Forbid();
			PutMsg( win -> UserPort, (struct Message *) msg);
			Permit();
		}
	}
}

void send_closeWindow(struct Window *win)
{
	if (win -> IDCMPFlags & IDCMP_CLOSEWINDOW)
	{
		struct IntuiMessage *msg;
		msg = (struct IntuiMessage *) AllocSysObjectTags(ASOT_MESSAGE,
			ASOMSG_Size, sizeof(struct IntuiMessage),
			ASOMSG_ReplyPort, reply_port,
			TAG_DONE);

		if (msg)
		{
			msg -> Class = IDCMP_CLOSEWINDOW;
			msg -> IDCMPWindow = win;

			Forbid();
			PutMsg( win -> UserPort, (struct Message *) msg);
			Permit();
		}
	}
}


void get_replymsg_and_delete(  )
{
	struct IntuiMessage *m;

	m = (struct IntuiMessage *) GetMsg( reply_port );
	while (m)
	{
		FreeSysObject(ASOT_MESSAGE, m);
		m = (struct IntuiMessage *) GetMsg( reply_port );
	}
}

void cleanup_engine( struct emuIntuitionContext *c )
{
	struct IntuiMessage *m;

	close_timer_context( &c -> tc );

	if (c->win)		// engine window...
	{
		// stop intutuion from sending new messages, we can't set IDCMP to 0, becouse intutuion will delete the MsgPort.
		ModifyIDCMP( c->win, IDCMP_CLOSEWINDOW );

		m = (struct IntuiMessage *) GetMsg( c->win->UserPort );
		while (m)
		{
			ReplyMsg( (struct Message *) m );
			m = (struct IntuiMessage *) GetMsg( c->win->UserPort );
		}
	
		CloseWindow( c->win );
		c->win = NULL;
	}

	if (reply_port)
	{
		get_replymsg_and_delete( c );
		FreeSysObject(ASOT_PORT, reply_port);
		reply_port = NULL;
	}

	if (c->dest_bitmap) 
	{
		FreeBitMap( c->dest_bitmap );
		c->dest_bitmap = NULL;
	}
}

void emuEngine()
{
	struct emuIntuitionContext c;
	ULONG win_mask = 0;
	ULONG mask_reply_port = 0;
	struct IntuiMessage *m;

	bool no_screens = false;

	bzero( &c.tc , sizeof(struct TimerContext) );

	c.win = OpenWindowTags( NULL, 
			WA_InnerWidth, 640,
			WA_InnerHeight, 480,
			WA_DragBar, TRUE,
			WA_CloseGadget, TRUE,

			WA_MinWidth,	320,
		 	WA_MinHeight,	200,	

			WA_MaxWidth,~0,
			WA_MaxHeight,	~0,

			WA_IDCMP, 
				IDCMP_CLOSEWINDOW |
				IDCMP_MOUSEBUTTONS | 
				IDCMP_MOUSEMOVE |
				IDCMP_INTUITICKS |
				IDCMP_RAWKEY |
				IDCMP_VANILLAKEY |
				IDCMP_GADGETUP
			,
			WA_RMBTrap, true,
			WA_ReportMouse, true,
			WA_DragBar, true,
			WA_DepthGadget, true,
			WA_SizeGadget, TRUE,
			WA_SizeBBottom, TRUE,
			TAG_END);

	if (!c.win) return ;

	c.dest_bitmap =AllocBitMap( 640, 480, 32, BMF_DISPLAYABLE, c.win ->RPort -> BitMap);

	if (!c.dest_bitmap) 
	{
		cleanup_engine( &c );
		return ;
	}

	InitRastPort( &c.local_rp );
	c.local_rp.BitMap = c.dest_bitmap;

	SetFont( &c.local_rp, default_font );

	if (open_timer_context( &c.tc)== false)
	{
		cleanup_engine( &c );
		return ;
	}

	win_mask = c.win -> UserPort ? 1 << c.win -> UserPort ->mp_SigBit : 0;

	reply_port = (APTR) AllocSysObjectTags(ASOT_PORT, TAG_DONE);

	mask_reply_port = 1L << reply_port -> mp_SigBit;

	do
	{
		ULONG host_w,host_h;
		ULONG host_mx,host_my;
		ULONG sig = Wait( win_mask | c.tc.timer_mask | SIGBREAKF_CTRL_C | mask_reply_port);

		if (sig & SIGBREAKF_CTRL_C)	break;

		if (sig & mask_reply_port) get_replymsg_and_delete();

		if (sig  & win_mask )
		{
			struct IntuiMessage *m;

			MutexObtain(video_mutex);

			c.src = current_fake_screen();

			host_w = c.win -> Width;
			host_w -= c.win -> BorderLeft;
			host_w -= c.win -> BorderRight;

			host_h = c.win -> Height;
			host_h -= c.win -> BorderTop;
			host_h -= c.win -> BorderBottom;

			m = (struct IntuiMessage *) GetMsg( c.win -> UserPort );
			while (m)
			{
				if (m -> Class)
				{
					switch (m -> Class)
					{
						case IDCMP_CLOSEWINDOW:

							Signal( host_task, 1L << host_sig);
							break;
					}
				}

				if (c.src)
				{
					bool has_active_win = window_open(c.src,active_win);
						
					switch (m -> Class)
					{
						case IDCMP_MOUSEMOVE:

							host_mx = c.win -> WScreen -> MouseX - c.win -> LeftEdge - c.win -> BorderLeft;
							host_my = c.win -> WScreen -> MouseY - c.win -> TopEdge - c.win -> BorderTop;

							c.src -> MouseX = host_mx * c.src -> Width / host_w;
							c.src -> MouseY = host_my * c.src -> Height / host_h;

							update_fake_window_mouse_xy(c.src)	;
						
							switch (mouse_state)
							{
								case size_action:
										size_window( c.src );
										break;

								case drag_action:
										drag_window( c.src );
										break;

								default:
										if (has_active_win) send_mouse_move( active_win , m );
										break;
							}
							break;

						case IDCMP_VANILLAKEY:
							FPrintf( output, "IDCMP_VANILLAKEY\n");
							if (has_active_win) send_copy( active_win, m );	
							break;

						case IDCMP_RAWKEY:
							FPrintf( output, "IDCMP_RAWKEY\n");
							if (has_active_win) send_copy( active_win, m );	
							break;

						case IDCMP_INTUITICKS:

							if (has_active_win) send_INTUITICKS( active_win );					
							break;

						case IDCMP_MOUSEBUTTONS:

							FPrintf( output, "IDCMP_MOUSEBUTTONS\n");

							if ( m->Code & IECODE_UP_PREFIX)
							{
								switch (mouse_state)
								{
									case close_action:
										if (has_active_win) send_closeWindow( active_win );
										break;

									case wdepth_action:
										wdepth_window( c.src );
										break;

									default:
										if (has_active_win) send_copy( active_win , m );
										break;
								}

								mouse_state = 0;
							}
							else
							{
								FPrintf( output, "mouse_state: %lx\n", mouse_state);

								switch (mouse_state)
								{
									case no_action:
										if (WindowClick( c.src ) == false)
										{
											if (has_active_win) send_copy( active_win , m );
										}
										break;
								}
							}
							break;

					}
				}

				ReplyMsg( (struct Message *) m );
				m = (struct IntuiMessage *) GetMsg( c.win -> UserPort );
			}

			MutexRelease(video_mutex);
		}

		if (sig & c.tc.timer_mask)
		{
			MutexObtain(video_mutex);

			c.src = current_fake_screen();
			if (c.src)
			{
				no_screens = false;

				update_argb_lookup( c.src -> ViewPort.ColorMap );

				ham.a = 0xFF;

				draw_screen( &c );
 				comp_window_update( c.src, c.dest_bitmap, c.win);
			}
			else
			{
				struct Screen src;
				const char info[]= "No AGA Screens open";

				src.Width = 640;
				src.Height = 480;
				src.ViewPort.DWidth = src.Width;
				src.ViewPort.DHeight = src.Height;

				if (no_screens == false)
				{
					RectFillColor( &c.local_rp,0,0,src.Width,src.Height, 0xFF000000);

					SetRPAttrs( &c.local_rp,  
						RPTAG_APenColor, 0xFFFFFFFF,
						RPTAG_BPenColor, 0xFF000000,
						TAG_END);

					Move( &c.local_rp, 20,20 );
					Text( &c.local_rp, info, strlen(info) );
					
				}

				no_screens = true;
 				comp_window_update( &src, c.dest_bitmap, c.win);
			}

			MutexRelease(video_mutex);

			reset_timer( c.tc.timer_io );
		}

	} while (!quit);

	cleanup_engine( &c );
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
/*
		COMPTAG_OffsetX,    msg->Bounds.MinX - (msg->OffsetX - hookData->offsetX),
		COMPTAG_OffsetY,    msg->Bounds.MinY - (msg->OffsetY - hookData->offsetY),
*/
		COMPTAG_OffsetX,    msg->Bounds.MinX - (msg->OffsetX - msg->Bounds.MinX),
		COMPTAG_OffsetY,    msg->Bounds.MinY - (msg->OffsetY - msg->Bounds.MinY),
		COMPTAG_DestX,      msg->Bounds.MinX,
		COMPTAG_DestY,      msg->Bounds.MinY,
		COMPTAG_DestWidth,  msg->Bounds.MaxX - msg->Bounds.MinX + 1,
		COMPTAG_DestHeight, msg->Bounds.MaxY - msg->Bounds.MinY + 1,
		COMPTAG_Flags,      COMPFLAG_SrcFilter | COMPFLAG_IgnoreDestAlpha | COMPFLAG_HardwareOnly,
		TAG_END);

	return 0;
}


void comp_window_update( struct Screen *src, struct BitMap *bitmap, struct Window *win)
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

	hookData.srcWidth = src -> ViewPort.DWidth;
	hookData.srcHeight = src -> Height < src -> ViewPort.DHeight ? src -> Height : src -> ViewPort.DHeight;

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



