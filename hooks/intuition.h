
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

extern VOID ppc_func_RefreshGList (struct IntuitionIFace *Self, struct Gadget * gadgets, struct Window * window, struct Requester * requester, WORD numGad);
extern UWORD ppc_func_AddGList(struct IntuitionIFace *Self, struct Window * window, struct Gadget * gadget, UWORD position, WORD numGad, struct Requester * requester);
extern VOID ppc_func_MoveScreen(struct IntuitionIFace *Self, struct Screen * screen, WORD dx, WORD dy);
extern VOID ppc_func_ScreenPosition(struct IntuitionIFace *Self, struct Screen * screen, ULONG flags, LONG x1, LONG y1, LONG x2, LONG y2);
VOID ppc_func_MoveScreen(struct IntuitionIFace *Self, struct Screen * screen, WORD dx, WORD dy);
void ppc_func_SetWindowTitles( struct IntuitionIFace *Self, struct Window *w, const char *winStr, const char *srcStr  );
struct ScreenBuffer * ppc_func_AllocScreenBuffer (struct IntuitionIFace *Self, struct Screen * sc, struct BitMap * bm, ULONG flags);
void ppc_func_SizeWindow( struct IntuitionIFace *Self, struct Window *w, LONG dx, LONG dy  );
void ppc_func_MoveWindow( struct IntuitionIFace *Self, struct Window *w, LONG dx, LONG dy  );
void ppc_func_ActivateWindow( struct IntuitionIFace *Self, struct Window *w );
void ppc_func_CloseWindow( struct IntuitionIFace *Self, struct Window *w );
struct Window * ppc_func_OpenWindowTagList (struct IntuitionIFace *Self, const struct NewWindow * newWindow, const struct TagItem * tagList);
void ppc_func_CloseScreen( struct IntuitionIFace *Self, struct Screen *screen );
void ppc_func_ScreenToBack( struct IntuitionIFace *Self, struct Screen *screen );
void ppc_func_ScreenToFront( struct IntuitionIFace *Self, struct Screen *screen );
ULONG ppc_func_ChangeScreenBuffer(struct IntuitionIFace *Self, struct Screen * s, struct ScreenBuffer * sb);
void ppc_func_FreeScreenBuffer( struct IntuitionIFace *Self, struct Screen *s, struct ScreenBuffer *sb );
struct Screen * ppc_func_OpenScreenTagList(struct IntuitionIFace *Self, const struct NewScreen * newScreen, const struct TagItem * tagList);

