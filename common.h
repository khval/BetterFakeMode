
extern bool monitor;
extern BPTR output;

#ifdef __USE_INLINE__
#define new_struct(x) 	(struct x *) AllocVecTags( sizeof(struct x), AVT_Type, MEMF_SHARED,AVT_ClearWithValue, 0, TAG_END)
#else
#define new_struct(x) 	(struct x *) IExec->AllocVecTags( sizeof(struct x), AVT_Type, MEMF_SHARED,AVT_ClearWithValue, 0, TAG_END)
#endif

extern struct Screen * _new_fake_OpenScreenTagList( const struct NewScreen * newScreen, const struct TagItem * tagList);
extern void _delete_fake_screen( struct Screen *s );
extern struct Window * fake_OpenWindowTagList ( const struct NewWindow * newWindow, const struct TagItem * tagList);
extern struct Window * fake_CloseWindow ( struct Window * window);


