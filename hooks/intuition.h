
extern void * old_ppc_func_RefreshGList;
extern void * old_ppc_func_AddGList;
extern void * old_ppc_func_ScreenPosition;
extern void * old_ppc_func_MoveScreen;

extern VOID ppc_func_RefreshGList (struct IntuitionIFace *Self, struct Gadget * gadgets, struct Window * window, struct Requester * requester, WORD numGad);
extern UWORD ppc_func_AddGList(struct IntuitionIFace *Self, struct Window * window, struct Gadget * gadget, UWORD position, WORD numGad, struct Requester * requester);
extern VOID ppc_func_MoveScreen(struct IntuitionIFace *Self, struct Screen * screen, WORD dx, WORD dy);
extern VOID ppc_func_ScreenPosition(struct IntuitionIFace *Self, struct Screen * screen, ULONG flags, LONG x1, LONG y1, LONG x2, LONG y2);

