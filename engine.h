
struct TimerContext
{
	struct MsgPort *timer_port ;
	struct TimeRequest *timer_io ;
	bool its_open;
	ULONG timer_mask ;
};

struct emuIntuitionContext
{
	struct RastPort local_rp;
	struct TimerContext tc;
	struct Screen *src;
	struct Window *win;
	struct BitMap *dest_bitmap;
};


typedef unsigned char u8;

void draw_screen( struct emuIntuitionContext *c  );

void dump_screen();

extern int frame_skip;
extern int line_skip;

