
struct debug_tag_info
{
	ULONG tag;
	const char *name; 
};

extern struct debug_tag_info win_info_tags[];
extern struct debug_tag_info screen_info_tags[];

extern bool monitor;
extern BPTR output;


#ifdef __USE_INLINE__
#define new_struct(x) 	(struct x *) AllocVecTags( sizeof(struct x), AVT_Type, MEMF_SHARED,AVT_ClearWithValue, 0, TAG_END)
#else
#define new_struct(x) 	(struct x *) IExec->AllocVecTags( sizeof(struct x), AVT_Type, MEMF_SHARED,AVT_ClearWithValue, 0, TAG_END)
#endif

extern APTR video_mutex;
extern struct TextFont *default_font;
extern struct Window *active_win;

extern struct Screen * _new_fake_OpenScreenTagList( const struct NewScreen * newScreen, const struct TagItem * tagList);
extern void _delete_fake_screen( struct Screen *s );

extern void box(struct RastPort *rp,int x0,int y0,int x1, int y1);

extern void RenderGadget(struct RastPort *rp, struct Gadget *g);
extern void RenderGadgets(struct RastPort *rp, struct Gadget *g);
extern void RenderWindow(struct Window *win);

extern void no_block_MoveWindow ( struct Window *win, LONG dx, LONG dy );
extern void no_block_SizeWindow ( struct Window *win, LONG dx, LONG dy );
extern void no_block_ActivateWindow( struct Window *w );

