
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

extern struct TextFont *default_font;

extern struct Screen * _new_fake_OpenScreenTagList( const struct NewScreen * newScreen, const struct TagItem * tagList);
extern void _delete_fake_screen( struct Screen *s );

extern struct Window * fake_OpenWindowTagList ( const struct NewWindow * newWindow, const struct TagItem * tagList);
extern struct Window * fake_CloseWindow ( struct Window * window);

extern struct ScreenBuffer * fake_AllocScreenBuffer ( struct Screen * sc, struct BitMap * bm, ULONG flags);
extern ULONG fake_ChangeScreenBuffer ( struct Screen * sc, struct ScreenBuffer * bm);

