
extern void fake_MoveWindow( struct Window *w, LONG dx, LONG dy  );
extern void fake_SizeWindow( struct Window *w, LONG dx, LONG dy  );
extern void fake_ActivateWindow( struct Window *w );
extern struct Window * fake_OpenWindowTagList ( const struct NewWindow * newWindow, const struct TagItem * tagList);
extern struct Window * fake_CloseWindow ( struct Window * window);
extern struct ScreenBuffer * fake_AllocScreenBuffer ( struct Screen * sc, struct BitMap * bm, ULONG flags);
extern ULONG fake_ChangeScreenBuffer ( struct Screen * sc, struct ScreenBuffer * bm);
extern void _delete_fake_screen( struct Screen *s );
extern struct Screen * _new_fake_OpenScreenTagList( const struct NewScreen * newScreen, const struct TagItem * tagList);
extern void fake_SetWindowTitles( struct Window *win, const char *winStr, const char *srcStr );
extern void fake_FreeScreenBuffer( struct Screen *s, struct ScreenBuffer *sb );
extern LONG fake_SetWindowAttrsA( struct Window *win, struct TagItem *taglist );
extern LONG fake_SetWindowAttr( struct Window * win, ULONG attr, APTR data, ULONG size);

