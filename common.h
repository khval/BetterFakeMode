
extern bool monitor;
extern BPTR output;


#ifdef __USE_INLINE__
#define new_struct(x) 	(struct x *) AllocVecTags( sizeof(struct x), AVT_Type, MEMF_SHARED,AVT_ClearWithValue, 0, TAG_END)
#else
#define new_struct(x) 	(struct x *) IExec->AllocVecTags( sizeof(struct x), AVT_Type, MEMF_SHARED,AVT_ClearWithValue, 0, TAG_END)
#endif

extern struct Screen *_new_fake_screen(int Width, int Height, int Depth);
extern void _delete_fake_screen( struct Screen *s );

