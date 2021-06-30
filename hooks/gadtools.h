
extern void * old_ppc_func_GetVisualInfo;
extern void * old_ppc_func_FreeVisualInfo;
extern void * old_ppc_func_CreateGadgetA;
extern void * old_ppc_func_GT_GetIMsg;
extern void * old_ppc_func_GT_ReplyIMsg;

APTR				ppc_func_GetVisualInfo(struct GadToolsIFace *Self, struct Screen * screen, const struct TagItem * taglist);
void					ppc_func_FreeVisualInfo(struct GadToolsIFace *Self, APTR ptr);
struct Gadget *			ppc_func_CreateGadgetA(struct GadToolsIFace *Self,ULONG kind,	struct Gadget * gad,	const struct NewGadget * ng,	const struct TagItem * taglist);
struct IntuiMessage *	ppc_func_GT_GetIMsg (struct GadToolsIFace *Self, struct MsgPort * iport);
void					ppc_func_GT_ReplyIMsg (struct GadToolsIFace *Self, struct IntuiMessage *msg);

