
extern void * old_ppc_func_RefreshGList;
extern void * old_ppc_func_AddGList;
extern void * old_ppc_func_ScreenPosition;
extern void * old_ppc_func_MoveScreen;

extern void *  old_ppc_func_OpenScreenTagList;
extern void *  old_ppc_func_CloseScreen;
extern void *  old_ppc_func_ScreenToFront;
extern void *  old_ppc_func_ScreenToBack;
extern void *  old_ppc_func_OpenWindowTagList;
extern void *  old_ppc_func_CloseWindow;
extern void *  old_ppc_func_AllocScreenBuffer;
extern void *  old_ppc_func_FreeScreenBuffer;
extern void *  old_ppc_func_ChangeScreenBuffer;
extern void *  old_ppc_func_MoveWindow;
extern void *  old_ppc_func_SizeWindow;
extern void *  old_ppc_func_SetWindowTitles;
extern void *  old_ppc_func_ActivateWindow;
extern void *  old_ppc_func_GetBitMapAttr;
extern void *  old_ppc_func_MoveScreen;
extern void *  old_ppc_func_ScreenPosition;
extern void *  old_ppc_func_SetWindowAttrsA;
extern void *  old_ppc_func_SetWindowAttr;

extern VOID ppc_func_RefreshGList (struct IntuitionIFace *Self, struct Gadget * gadgets, struct Window * window, struct Requester * requester, WORD numGad);
extern UWORD ppc_func_AddGList(struct IntuitionIFace *Self, struct Window * window, struct Gadget * gadget, UWORD position, WORD numGad, struct Requester * requester);
extern VOID ppc_func_MoveScreen(struct IntuitionIFace *Self, struct Screen * screen, WORD dx, WORD dy);
extern VOID ppc_func_ScreenPosition(struct IntuitionIFace *Self, struct Screen * screen, ULONG flags, LONG x1, LONG y1, LONG x2, LONG y2);
extern VOID ppc_func_MoveScreen(struct IntuitionIFace *Self, struct Screen * screen, WORD dx, WORD dy);
extern void ppc_func_SetWindowTitles( struct IntuitionIFace *Self, struct Window *w, const char *winStr, const char *srcStr  );
extern struct ScreenBuffer * ppc_func_AllocScreenBuffer (struct IntuitionIFace *Self, struct Screen * sc, struct BitMap * bm, ULONG flags);
extern void ppc_func_SizeWindow( struct IntuitionIFace *Self, struct Window *w, LONG dx, LONG dy  );
extern void ppc_func_MoveWindow( struct IntuitionIFace *Self, struct Window *w, LONG dx, LONG dy  );
extern void ppc_func_ActivateWindow( struct IntuitionIFace *Self, struct Window *w );
extern void ppc_func_CloseWindow( struct IntuitionIFace *Self, struct Window *w );
extern struct Window * ppc_func_OpenWindowTagList (struct IntuitionIFace *Self, const struct NewWindow * newWindow, const struct TagItem * tagList);
extern void ppc_func_CloseScreen( struct IntuitionIFace *Self, struct Screen *screen );
extern void ppc_func_ScreenToBack( struct IntuitionIFace *Self, struct Screen *screen );
extern void ppc_func_ScreenToFront( struct IntuitionIFace *Self, struct Screen *screen );
extern ULONG ppc_func_ChangeScreenBuffer(struct IntuitionIFace *Self, struct Screen * s, struct ScreenBuffer * sb);
extern void ppc_func_FreeScreenBuffer( struct IntuitionIFace *Self, struct Screen *s, struct ScreenBuffer *sb );
extern struct Screen * ppc_func_OpenScreenTagList(struct IntuitionIFace *Self, const struct NewScreen * newScreen, const struct TagItem * tagList);
extern LONG ppc_func_SetWindowAttrsA (struct IntuitionIFace *Self, struct Window * win, struct TagItem * taglist);
extern LONG ppc_func_SetWindowAttr (struct IntuitionIFace *Self, struct Window * win,ULONG attr, APTR data, ULONG size);

