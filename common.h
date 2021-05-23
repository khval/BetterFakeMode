
extern bool monitor;
extern BPTR output;

// 13 because its not taken, move it up if there is conflict.
// BMB_HIJACKED might work, but don't wont patch effect other things.

#define BMB_FAKE_BITMAP 13
#define BMF_FAKE_BITMAP (1L<<BMB_FAKE_BITMAP)

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

extern struct Window * fake_OpenWindowTagList ( const struct NewWindow * newWindow, const struct TagItem * tagList);
extern struct Window * fake_CloseWindow ( struct Window * window);

extern struct ScreenBuffer * fake_AllocScreenBuffer ( struct Screen * sc, struct BitMap * bm, ULONG flags);
extern ULONG fake_ChangeScreenBuffer ( struct Screen * sc, struct ScreenBuffer * bm);

extern void fake_MoveWindow( struct Window *w, LONG dx, LONG dy  );
extern void fake_SizeWindow( struct Window *w, LONG dx, LONG dy  );
extern void fake_ActivateWindow( struct Window *w );

extern void box(struct RastPort *rp,int x0,int y0,int x1, int y1);

extern void RenderGadget(struct RastPort *rp, struct Gadget *g);
extern void RenderGadgets(struct RastPort *rp, struct Gadget *g);
extern void RenderWindow(struct Window *win);

extern void no_block_MoveWindow ( struct Window *win, LONG dx, LONG dy );
extern void no_block_SizeWindow ( struct Window *win, LONG dx, LONG dy );
extern void no_block_ActivateWindow( struct Window *w );

