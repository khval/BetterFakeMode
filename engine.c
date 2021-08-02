

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

#include <intuition/imageclass.h>
#include <intuition/gadgetclass.h>

#include "common.h"
#include "engine.h"
#include "EngineTimer.h"
#include "helper/screen.h"
#include "init.h"


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

enum
{
	GID_SCREENDEPTH = 20,
	GID_FULLSCREEN,
	GID_ICONIFY
};

struct xy
{
	int x;
	int y;
};

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


union amiga_argb OCS_lookup[0x10000];

void init_ocs_lookup()
{
	int i;
	for (i=0;i<0x10000;i++)
	{
		OCS_lookup[i].a = 0xFF;
		OCS_lookup[i].r = ((i & 0xF00) >> 8) * 0x11;
		OCS_lookup[i].g = ((i & 0x0F0) >> 4) * 0x11;
		OCS_lookup[i].b = (i & 0x00F) * 0x11;
	}
}

struct xy tmp_mouse = {0,0};
struct xy delta_mouse = {0,0};

struct kIcon iconifyIcon = { NULL, NULL };
struct kIcon zoomIcon = { NULL, NULL };
struct kIcon screendepthicon = { NULL, NULL};

void send_button( struct Window *win,  struct IntuiMessage *source_msg, ULONG idcmp );

union amiga_argb palette[256];

uint64 qLookup_i64[256*256];
double *qLookup_d64 = (double *) qLookup_i64;

void comp_window_update( struct Screen *src, struct BitMap *bitmap, struct Window *win);

bool compare_IDCMP(ULONG mask, ULONG IDCMP1, ULONG IDCMP2)
{
	return ((mask & IDCMP1) == (mask & IDCMP2));
}

ULONG new_IDCMP( ULONG mask, ULONG old_IDCMP, ULONG new_IDCMP)
{
	return (old_IDCMP & ~mask) | (new_IDCMP & mask);
}

void initQLookup()
{
	uint32 *p;
	int i;

	// pixel [0,256],[0..256]

	for (i=0;i<(256*256);i++)
	{
		p = (uint32 *) (qLookup_d64+i);
		p[0] = 0xFF000000 | palette[(i & 0xFF00) >> 8].argb;
		p[1] = 0xFF000000 | palette[(i & 0x00FF) ].argb;
	}
}


// update min index for a color, in quick lookup table...

void initQLookupColor( int max_colors, int color)
{
	uint32 *p;
	int i_1st =color << 8;
	int i_2en;
	int i;

	for (i_2en=0;i_2en<max_colors;i_2en++)
	{
		i = i_1st | i_2en;

		p = (uint32 *) (qLookup_d64+i);
		p[0] = 0xFF000000 | palette[ color ].argb;
		p[1] = 0xFF000000 | palette[ i_2en ].argb;
	}

	i_2en = color;

	for (i_1st= 0;i_1st<max_colors;i_1st++)
	{
		i = (i_1st << 8) | i_2en;

		p = (uint32 *) (qLookup_d64+i);
		p[0] = 0xFF000000 | palette[ i_1st ].argb;
		p[1] = 0xFF000000 | palette[ color ].argb;
	}

}

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

	initQLookup();
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

void draw_bits_argb32( unsigned char *dest_ptr, unsigned char *b)
{
	uint32 *d_argb = (uint32 *)	(dest_ptr);

	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb++ = palette[ *b++ ].argb ;
	*d_argb = palette[ *b ].argb ;
}

void draw_bits_argb64( unsigned char *dest_ptr, unsigned short *b)
{
	double *d_argb_argb = (double *)	(dest_ptr);

	*d_argb_argb++ = qLookup_d64[ *b++ ] ;
	*d_argb_argb++ = qLookup_d64[ *b++ ] ;
	*d_argb_argb++ = qLookup_d64[ *b++ ] ;
	*d_argb_argb = qLookup_d64[ *b ] ;
}

void draw_bits_argb_ham6( unsigned char *dest_ptr, unsigned char *b )
{
	uint32 *d_argb = (uint32 *)	(dest_ptr);

	*d_argb++ = ham6( *b++ ); // 0
	*d_argb++ = ham6( *b++ ); // 1
	*d_argb++ = ham6( *b++ ); // 2
	*d_argb++ = ham6( *b++ ); // 3
	*d_argb++ = ham6( *b++ ); // 4
	*d_argb++ = ham6( *b++ ); // 5
	*d_argb++ = ham6( *b++ ); // 6
	*d_argb = ham6( *b ); // 7
}

void draw_bits_argb_ham8( unsigned char *dest_ptr, unsigned char *b)
{
	uint32 *d_argb = (uint32 *)	(dest_ptr);	

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


void draw_hpixels(
			uint64 (*planar_routine) (unsigned char *ptr, int SizeOfPlane) ,
			void (*draw_bits) ( unsigned char *dest_ptr, unsigned char *b) ,
			int SizeOfPlane, 
			int x, 
			unsigned char *scr_ptr_y_start, 
			unsigned char *scr_ptr_y_end, 
			unsigned char *dest_ptr )
{
	uint64 data = 0;
	unsigned char *scr_ptr;
	x = 0;
	for (scr_ptr=scr_ptr_y_start;scr_ptr<scr_ptr_y_end;scr_ptr++)
	{
		data = planar_routine( scr_ptr, SizeOfPlane );
		draw_bits( dest_ptr, (unsigned char *) &data );
		dest_ptr+=(8*4);		// 8 pixels 4 bytes per pixel
	}
}


void draw_screen( struct emuIntuitionContext *c) 
{
	struct BitMap *bm = c -> scr -> RastPort.BitMap;
	struct BitMap *dest_bm = c -> dest_bitmap;

	int SizeOfPlane;
	int x,y;
	int bpr;
	unsigned char *scr_ptr;
	unsigned char *scr_ptr_y_start;
	unsigned char *scr_ptr_y_end;
	uint64 data = 0;
	uint max_height;

	ULONG dest_format;
	unsigned char *dest_ptr_image;
	unsigned char *dest_ptr;
	unsigned int dest_bpr;
	APTR lock;

	uint64 (*planar_routine) (unsigned char *ptr, int SizeOfPlane) = NULL;
	void (*draw_bits) ( unsigned char *dest_ptr, unsigned char *b) = NULL;

	bpr = bm -> BytesPerRow;
	SizeOfPlane = bm -> BytesPerRow * bm -> Rows;
	planar_routine = planar_routines[ bm -> Depth ];

	switch ( (c -> scr -> ViewPort.Modes & 0x800) | bm -> Depth )
	{
		case 0x0806:	draw_bits = draw_bits_argb_ham6;		break;
		case 0x0808:	draw_bits = draw_bits_argb_ham8;		break;
		default:		draw_bits = draw_bits_argb64;			break;
	}

	lock = LockBitMapTags( dest_bm,
			LBM_PixelFormat, &dest_format,
			LBM_BytesPerRow, &dest_bpr,
			LBM_BaseAddress, &dest_ptr_image,
			TAG_END	 );

	max_height = c -> scr -> ViewPort.DHeight;
	if (bm -> Rows < max_height) max_height = bm -> Rows;

	for (y=0;y<max_height; y++)
	{
		ham.argb = 0xFF000000;

		scr_ptr_y_start = bm -> Planes[0] + bpr*y;
		scr_ptr_y_end = scr_ptr_y_start + bpr;
		dest_ptr = dest_ptr_image + (dest_bpr*y);

		x = 0;
		for (scr_ptr=scr_ptr_y_start;scr_ptr<scr_ptr_y_end;scr_ptr++)
		{
			data = planar_routine( scr_ptr, SizeOfPlane );
			draw_bits( dest_ptr, (unsigned char *) &data );
			dest_ptr+=(8*4);		// 8 pixels 4 bytes per pixel
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
	wdepth_action,
	gadget_action
};

LONG mouse_state = 0;
struct Window *active_win=NULL;
LONG clicked_x,clicked_y;
struct Gadget *clicked_gadget;

bool ClickButtons( struct Window *win, struct IntuiMessage *m )
{
	ULONG x0,y0,x1,y1;
	LONG mx,my;
	struct Gadget *g;

	clicked_gadget = NULL;
	clicked_x = 0;
	clicked_y = 0;

	mouse_state = no_action;
	mx = win -> MouseX;
	my = win -> MouseY;

	for (g = win -> FirstGadget; g ; g = g -> NextGadget )
	{
		x0 = g -> LeftEdge;
		y0 = g -> TopEdge;
		x1 = g -> LeftEdge +  g -> Width;
		y1 = g -> TopEdge + g -> Height;
		
		if (
			( mx >= x0 ) && ( mx < x1) &&
			( my >= y0) && ( my < y1)
		)
		{
			FPrintf( output, "Clicked button\n");
			FPrintf( output, "X: %ld>%ld<%ld\n",x0,mx,x1);
			FPrintf( output, "Y: %ld>%ld<%ld\n",y0,my,y1);

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
					mouse_state = gadget_action;
					clicked_gadget = g;
					clicked_x = mx;
					clicked_y = my;

					send_button( win , m, IDCMP_GADGETDOWN );

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

bool WindowClick( struct Screen *src, struct IntuiMessage *m )
{
	ULONG mx,my;
	struct Window *win;
	struct Window *clicked_win;
	UWORD last_clicked_layer_priority = ~0;

	FPrintf( output, "%s:%ld: Screen %08lx\n",__FUNCTION__,__LINE__, src);

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
	 	return ClickButtons( clicked_win, m );
		
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

void copy_msg(struct IntuiMessage *source_msg,struct IntuiMessage *msg)
{
	msg -> Class = source_msg -> Class;
	msg -> Code = source_msg -> Code;
	msg -> Qualifier = source_msg -> Qualifier;
	msg -> Seconds = source_msg -> Seconds;
	msg -> Micros = source_msg -> Micros;
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
			copy_msg(source_msg,msg);
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
			copy_msg(source_msg,msg);
			msg -> IDCMPWindow = win;

			if (win -> IDCMPFlags & IDCMP_DELTAMOVE)
			{
				msg -> MouseX = delta_mouse.x;
				msg -> MouseY = delta_mouse.y;
			}
			else
			{
				msg -> MouseX = win -> MouseX;
				msg -> MouseY = win -> MouseY;
			}

			Forbid();
			PutMsg( win -> UserPort, (struct Message *) msg);
			Permit();
		}
	}
}

void send_button( struct Window *win,  struct IntuiMessage *source_msg, ULONG idcmp )
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);


	if (win -> IDCMPFlags & idcmp)
	{
		struct IntuiMessage *msg;
		msg = (struct IntuiMessage *) AllocSysObjectTags(ASOT_MESSAGE,
			ASOMSG_Size, sizeof(struct IntuiMessage),
			ASOMSG_ReplyPort, reply_port,
			TAG_DONE);

		if (msg)
		{
			copy_msg(source_msg,msg);
			msg -> IDCMPWindow = win;
			msg -> MouseX = win -> MouseX;
			msg -> MouseY = win -> MouseY;
			msg -> Code = clicked_gadget -> GadgetID;
			msg -> IAddress = clicked_gadget;
			msg -> Class = idcmp;

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
	

		dispose_icon( c->win,  &zoomIcon );
		dispose_icon( c->win,  &iconifyIcon );
		dispose_icon( c->win,  &screendepthicon );

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

void do_ScreenDepth()
{
	struct Screen *scr = current_fake_screen();
	fake_screen_to_back( scr );
}

void update_screen(struct emuIntuitionContext *c, ULONG host_w, ULONG host_h)
{
	struct xy new_mouse;
	ULONG host_mx,host_my;
	struct Window *win = c -> win;
	struct Screen *src = c -> scr;

	host_mx = win -> WScreen -> MouseX - win -> LeftEdge - win -> BorderLeft;
	host_my = win -> WScreen -> MouseY - win -> TopEdge - win -> BorderTop;

	new_mouse.x = host_mx * src -> Width / host_w;
	new_mouse.y = host_my * src -> Height / host_h;

	tmp_mouse.x  = new_mouse.x - src -> MouseX ;
	tmp_mouse.y  = new_mouse.y - src -> MouseY ;

	delta_mouse.x = (abs(tmp_mouse.x) < 100)  ? tmp_mouse.x * 2 : 0;
	delta_mouse.y = (abs(tmp_mouse.x) < 100)  ? tmp_mouse.y * 2 : 0;

	src -> MouseX = new_mouse.x;
	src -> MouseY = new_mouse.y;
}

void set_default_screen( struct Screen *src )
{
	src -> Width = 640;
	src -> Height = 480;
	src -> ViewPort.DWidth = src -> Width;
	src -> ViewPort.DHeight = src -> Height;
}

void draw_no_aga(struct Screen *src,struct RastPort *rp)
{
	const char info[]= "No AGA Screens open";
	RectFillColor( rp,0,0,src -> Width,src -> Height, 0xFF000000);

	SetRPAttrs( rp,  
		RPTAG_APenColor, 0xFFFFFFFF,
		RPTAG_BPenColor, 0xFF000000,
		TAG_END);

	Move( rp, 20,20 );
	Text( rp, info, strlen(info) );
}

void copMove( UWORD data, UWORD addr )
{
	switch (addr)
	{
		case 0x180: palette[0].argb = OCS_lookup[data].argb; break;
		case 0x182: palette[1].argb = OCS_lookup[data].argb; break;
		case 0x184: palette[2].argb = OCS_lookup[data].argb; break;
		case 0x186: palette[3].argb = OCS_lookup[data].argb; break;
		case 0x188: palette[4].argb = OCS_lookup[data].argb; break;
		case 0x18A: palette[5].argb = OCS_lookup[data].argb; break;
		case 0x18C: palette[6].argb = OCS_lookup[data].argb; break;
		case 0x18E: palette[7].argb = OCS_lookup[data].argb; break;
		case 0x190: palette[8].argb = OCS_lookup[data].argb; break;
		case 0x192: palette[9].argb = OCS_lookup[data].argb; break;
		case 0x194: palette[2].argb = OCS_lookup[data].argb; break;
		case 0x196: palette[3].argb = OCS_lookup[data].argb; break;
		case 0x198: palette[4].argb = OCS_lookup[data].argb; break;
		case 0x19A: palette[5].argb = OCS_lookup[data].argb; break;
		case 0x19C: palette[6].argb = OCS_lookup[data].argb; break;
		case 0x19E: palette[7].argb = OCS_lookup[data].argb; break;
		case 0x1A0: palette[8].argb = OCS_lookup[data].argb; break;
		case 0x1A2: palette[9].argb = OCS_lookup[data].argb; break;
	}
}


#define debug_copper 0


void drawCopIns( struct emuIntuitionContext *c, struct CopIns *cop, int cnt )
{

	struct BitMap *bm = c -> scr -> RastPort.BitMap;
	struct BitMap *dest_bm = c -> dest_bitmap;
	ULONG last_pos =0;
	ULONG new_pos = 0;
	LONG delta_pos;
	ULONG hpos,vpos;
	ULONG bpr;
	ULONG drawBytes;
	ULONG last_VWaitPos = 0, last_HWaitPos = 0;
	ULONG VWaitPos = 0, HWaitPos = 0;
	ULONG SizeOfPlane ;
	ULONG dest_bpr;
	ULONG dest_format;

	unsigned char *scr_ptr_y_start;
	unsigned char *scr_ptr_y_end;

	unsigned char *dest_ptr_image;
	unsigned char *dest_ptr;

#if debug_copper==1
	ULONG count =0;
	struct RastPort *rp = &(c -> local_rp);
#else
	APTR lock;
#endif

	uint64 (*planar_routine) (unsigned char *ptr, int SizeOfPlane) = NULL;
	void (*draw_bits) ( unsigned char *dest_ptr, unsigned char *b) = NULL;

	bpr = bm -> BytesPerRow;

	SizeOfPlane = bm -> BytesPerRow * bm -> Rows;
	planar_routine = planar_routines[ bm -> Depth ];

	switch ( (c -> scr -> ViewPort.Modes & 0x800) | bm -> Depth )
	{
		case 0x0806:	draw_bits = draw_bits_argb_ham6;		break;
		case 0x0808:	draw_bits = draw_bits_argb_ham8;		break;
		default:		draw_bits = draw_bits_argb32;			break;
	}


#if debug_copper==0

	lock = LockBitMapTags( dest_bm,
			LBM_PixelFormat, &dest_format,
			LBM_BytesPerRow, &dest_bpr,
			LBM_BaseAddress, &dest_ptr_image,
			TAG_END	 );
#else

	Printf("Render copper list\n");

#endif

	while (cnt --)
	{
		switch (cop -> OpCode)
		{
			case 0:		// move.
				copMove( cop -> u3.u4.u2.DestData, cop -> u3.u4.u1.DestAddr );
				break;

			case 1:		// wait..

				if ((cop -> u3.nxtlist == (void *) 0x271000FF ))	// END OF LIST....
				{
					HWaitPos = bm -> BytesPerRow ;
					VWaitPos = bm -> Rows -1;
				}
				else
				{
					HWaitPos = cop -> u3.u4.u2.HWaitPos;
					VWaitPos = cop -> u3.u4.u1.VWaitPos ;
				}

				last_pos = (last_VWaitPos * bpr) + last_HWaitPos;
				new_pos = VWaitPos * bpr + HWaitPos;

#if debug_copper==1
				FPrintf(output, "last_pixel_pos %ld, last_HWaitPos: %ld, last_VWaitPos: %ld\n",
					last_pos, last_HWaitPos, last_VWaitPos);

				FPrintf(output, "  to_pixel_pos %ld,   to_HWaitPos: %ld,   to_VWaitPos: %ld\n",
					new_pos, HWaitPos , VWaitPos);
#endif
				delta_pos = new_pos - last_pos;

				while (delta_pos>0)
				{
					hpos = last_pos % bpr;
					vpos = last_pos / bpr;

					drawBytes = hpos + delta_pos > bpr ? bpr :  delta_pos;	

#if debug_copper==1
					FPrintf(output, "last_pixel_pos %ld, delta_pos: %ld, drawBytes: %ld, v: %ld, h: %ld\n",
						last_pos,
						delta_pos,
						drawBytes,
						hpos,vpos);
					SetRPAttrs( rp,  RPTAG_APenColor, copColor[count & 3], TAG_END );	count ++;
#endif

					if (drawBytes>0)
					{
#if debug_copper==1
						Move(rp,hpos*8,vpos);
						Draw(rp,(hpos+drawBytes)*8, vpos);
#else
						scr_ptr_y_start = bm -> Planes[0] + (bpr * vpos) + hpos;
						scr_ptr_y_end = scr_ptr_y_start + drawBytes;
						
						dest_ptr = dest_ptr_image + (dest_bpr*vpos);

						draw_hpixels( planar_routine , draw_bits ,SizeOfPlane, hpos*8, scr_ptr_y_start, scr_ptr_y_end, dest_ptr );
#endif
						last_pos += drawBytes;
						delta_pos -= drawBytes;
					}
					else break;
				}

				last_VWaitPos = VWaitPos;
				last_HWaitPos = HWaitPos;
				break;
		}
		cop++;
	}

#if debug_copper==0
	UnlockBitMap( lock );
#endif

}


void draw_copper_screen( struct emuIntuitionContext *c )
{
	struct CopList  *cl;

	cl = c -> scr -> ViewPort.UCopIns -> FirstCopList;
	if ( cl )
	{
		drawCopIns( c, cl -> CopIns, cl -> Count );
	}
}

void emuEngine()
{
	struct emuIntuitionContext c;
	ULONG win_mask = 0;
	ULONG mask_reply_port = 0;
	bool no_screens = false;
	struct Screen *new_active_screen = NULL;
	ULONG GadgetID;

	init_ocs_lookup();

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
				IDCMP_GADGETUP |
				IDCMP_CLOSEWINDOW |
				IDCMP_MOUSEBUTTONS | 
				IDCMP_MOUSEMOVE |
				IDCMP_INTUITICKS |
				IDCMP_RAWKEY |
				IDCMP_VANILLAKEY |
				IDCMP_GADGETUP|
				IDCMP_ACTIVEWINDOW |
				IDCMP_INACTIVEWINDOW
			,
			WA_RMBTrap, true,
			WA_ReportMouse, true,
			WA_DragBar, true,
			WA_DepthGadget, true,
			WA_SizeGadget, TRUE,
			WA_SizeBBottom, TRUE,
			TAG_END);

	if (!c.win) return ;

	if (c.win)
	{
		struct DrawInfo *dri = GetScreenDrawInfo(c.win -> WScreen);

		if (dri)
		{
//			open_icon( c.win, dri, GUPIMAGE, GID_FULLSCREEN, &zoomIcon );
//			open_icon( c.win, dri, ICONIFYIMAGE, GID_ICONIFY, &iconifyIcon );
			open_icon( c.win, dri, DEPTHIMAGE, GID_SCREENDEPTH, &screendepthicon );
		}
	}

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
		bool has_active_win;
		ULONG host_w,host_h;
		ULONG sig = Wait( win_mask | c.tc.timer_mask | SIGBREAKF_CTRL_C | mask_reply_port);

		if (sig & SIGBREAKF_CTRL_C)	break;

		if (sig & mask_reply_port) get_replymsg_and_delete();

		if (sig  & win_mask )
		{
			struct IntuiMessage *m;

			MutexObtain(video_mutex);

			has_active_win = false;

			c.scr = current_fake_screen();
			if (c.scr)
			{
				has_active_win = window_open(c.scr,active_win);

				if (has_active_win == false)
				{
					active_win = c.scr -> FirstWindow;	// this is a hack....
				}
			}

			if (has_active_win)
			{
				if (compare_IDCMP( IDCMP_VANILLAKEY, c.win -> IDCMPFlags, active_win -> IDCMPFlags ) == FALSE)
				{
					ModifyIDCMP( c.win, new_IDCMP( IDCMP_VANILLAKEY, c.win -> IDCMPFlags, active_win -> IDCMPFlags) );
				}
			}

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
//					FPrintf( output, "Class: %08lx (%ld)\n", (int) m -> Class, (int) m -> Class);

					switch (m -> Class)
					{
						case IDCMP_CLOSEWINDOW:

							Signal( host_task, 1L << host_sig);
							break;

						case IDCMP_GADGETUP:

							FPrintf( output, "IDCMP_GADGETUP: %08lx (%ld)\n", (int) m -> Code, (int) m -> Code);

							GadgetID = ((struct Gadget *) ( m -> IAddress)) -> GadgetID ;

							switch(GadgetID)
							{
								case GID_SCREENDEPTH:
									FPrintf( output, "GID_SCREENDEPTH\n");
									do_ScreenDepth();
									break;
							}
							break;

						case IDCMP_ACTIVEWINDOW:

							frame_skip = 0;
							line_skip =  0;
							break;

						case IDCMP_INACTIVEWINDOW:

							frame_skip = 2;
							line_skip =  2;
							break;
					}
				}

				if (c.scr)
				{
					switch (m -> Class)
					{

						case IDCMP_MOUSEMOVE:

							update_screen( &c, host_w, host_h );
							update_fake_window_mouse_xy(c.scr)	;
						
							switch (mouse_state)
							{
								case size_action:
										size_window( c.scr );
										break;

								case drag_action:
										drag_window( c.scr );
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
										if (has_active_win) wdepth_window( c.scr );
										break;
									
									case gadget_action:
										if (has_active_win) send_button( active_win , m, IDCMP_GADGETUP );
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

								update_screen( &c, host_w, host_h );

								switch (mouse_state)
								{
									case no_action:
										if (WindowClick( c.scr, m ) == false)
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

			c.scr = current_fake_screen();
			if (c.scr)
			{
				no_screens = false;

				if (c.scr -> ViewPort.UCopIns)
				{
					update_argb_lookup( c.scr -> ViewPort.ColorMap );
					ham.a = 0xFF;
					draw_copper_screen( &c );
				} else {
					update_argb_lookup( c.scr -> ViewPort.ColorMap );
					ham.a = 0xFF;
					draw_screen( &c );
				}

 				comp_window_update( c.scr, c.dest_bitmap, c.win);
			}
			else
			{
				struct Screen src;
				set_default_screen( &src );
				if (no_screens == false) draw_no_aga( &src, &c.local_rp );
				no_screens = true;
 				comp_window_update( &src, c.dest_bitmap, c.win);
			}

			MutexRelease(video_mutex);

			WaitTOF();
			reset_timer( c.tc.timer_io );

			if (new_active_screen)
			{
				c.scr = new_active_screen;
				new_active_screen = NULL;
			}
		}

	} while (!quit);

	cleanup_engine( &c );
}


struct XYSTW_Vertex3D { 
float x, y; 
float s, t, w; 
}; 

typedef struct CompositeHookData_s {
	struct BitMap *scrBitMap; // The source bitmap
	int32 scrX,scrY;
	int32 scrWidth, scrHeight; // The source dimensions
	int32 offsetX, offsetY; // The offsets to the destination area relative to the window's origin
	int32 destWidth, destHeight;
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

//	int scr_width = 640;
//	int scr_height = 480;
	int DestWidth = msg->Bounds.MaxX - msg->Bounds.MinX;
	int DestHeight = msg->Bounds.MaxY - msg->Bounds.MinY;
	int offsetX = 0;
	int offsetY = 0;

	CompositeTags(
		COMPOSITE_Src, bm, rastPort->BitMap,

		COMPTAG_SrcWidth,   scr_width,
		COMPTAG_SrcHeight,  scr_height,
		COMPTAG_ScaleX, 	COMP_FLOAT_TO_FIX( (float) DestWidth /  (float) scr_width),
		COMPTAG_ScaleY, 	COMP_FLOAT_TO_FIX( (float) DestHeight / (float) scr_height),
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
			hookData->scrBitMap, 
			rastPort->BitMap,

		COMPTAG_SrcX,      abs(hookData->scrX),
		COMPTAG_SrcY,      abs(hookData->scrY),
		COMPTAG_SrcWidth,   hookData->scrWidth,
		COMPTAG_SrcHeight,  hookData->scrHeight,

		COMPTAG_ScaleX, 	hookData->scaleX,
		COMPTAG_ScaleY, 	hookData->scaleY,

		COMPTAG_OffsetX,    msg->Bounds.MinX - (msg->OffsetX - msg->Bounds.MinX),
		COMPTAG_OffsetY,    msg->Bounds.MinY - (msg->OffsetY - msg->Bounds.MinY),

		COMPTAG_DestX,      msg->Bounds.MinX,
		COMPTAG_DestY,      msg->Bounds.MinY,

		COMPTAG_DestWidth, hookData->destWidth,
		COMPTAG_DestHeight, hookData->destHeight ,

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

//	FPrintf( output, "DyOffset: %ld\n", src -> ViewPort.DyOffset); 

 	rect.MinX = win->BorderLeft;
 	rect.MinY = win->BorderTop;
 	rect.MaxX = win->Width - win->BorderRight ;
 	rect.MaxY = win->Height - win->BorderBottom ;

	hook.h_Entry = (HOOKFUNC) compositeHookFunc;
	hook.h_Data = &hookData;

	hookData.scrBitMap = bitmap;

//	I belive some games /demos might double buffer by moving the screen... 

	hookData.scrX = src -> ViewPort.DxOffset;
	hookData.scrY = src -> ViewPort.DyOffset;

	hookData.scrWidth = src -> ViewPort.DWidth;
	hookData.scrHeight = src -> Height < src -> ViewPort.DHeight ? src -> Height : src -> ViewPort.DHeight;

	hookData.offsetX = win->BorderLeft;
	hookData.offsetY = win->BorderTop;
	hookData.retCode = COMPERR_Success;

 	hookData.destWidth = rect.MaxX - rect.MinX + 1;
 	hookData.destHeight = rect.MaxY - rect.MinY + 1;

	hookData.scaleX = COMP_FLOAT_TO_FIX((float) hookData.destWidth / (float) hookData.scrWidth);
	hookData.scaleY = COMP_FLOAT_TO_FIX((float) hookData.destHeight / (float) hookData.scrHeight);

	LockLayer(0, RPort->Layer);
	DoHookClipRects(&hook, win->RPort, &rect);
	UnlockLayer( RPort->Layer);
}



