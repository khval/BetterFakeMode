
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "hooks/gadtools.h"
#include "helper/screen.h"
#include "common.h"

// We most create a bad VisualInfo, becouse gadtools, don't like fake windows.

extern struct MsgPort *reply_port;

APTR ppc_func_GetVisualInfo(struct GadToolsIFace *Self, struct Screen * screen, const struct TagItem * taglist)
{
	if (is_fake_screen( screen ))
	{
		FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
		return (APTR) 0xFA8EFA8E;	// this how we know... its fake..
	}
	else
	{
		return ((APTR (*)( struct GadToolsIFace *, struct Screen *, const struct TagItem *))
				old_ppc_func_GetVisualInfo) ( Self, screen, taglist);
	}

	return NULL;
}

// We most use this patch, to avoid bad VisualInfo from being freed. 

void ppc_func_FreeVisualInfo(struct GadToolsIFace *Self, APTR ptr)
{
	if (ptr == (APTR) 0xFA8EFA8E)
	{
		FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
	}
	else
	{
		return ((void (*)( struct GadToolsIFace *, APTR ptr))
				old_ppc_func_FreeVisualInfo) ( Self, ptr);
	}
}

// We most patch this, becouse of a bad VisualInfo.

struct Gadget * ppc_func_CreateGadgetA(struct GadToolsIFace *Self,ULONG kind,struct Gadget * gad,const struct NewGadget * ng,const struct TagItem * taglist)
{
	if ( ng -> ng_VisualInfo == (APTR) 0xFA8EFA8E)
	{
		FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
		return fake_CreateGadgetA ( kind,  gad, ng, taglist);
	}
	else
	{
		return ((struct Gadget * (*)( struct GadToolsIFace *,ULONG , struct Gadget *, const struct NewGadget *, const struct TagItem *))
				old_ppc_func_CreateGadgetA) ( Self, kind, gad, ng, taglist);
	}

	return NULL;
}

// Problem GT_GetIMsg, starts poking around in gadgets, this goes wrong...
// we most patch it.. (unless someone tells me what to do...)

struct IntuiMessage * ppc_func_GT_GetIMsg (struct GadToolsIFace *Self, struct MsgPort * iport)
{
	if (iport -> mp_Node.ln_Name)
	{
		if (strncmp(iport -> mp_Node.ln_Name,"FakeWindow",10)==0)
		{
			FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
			return (struct IntuiMessage *) GetMsg( iport );
		}
	}

	return ((struct IntuiMessage * (*)( struct GadToolsIFace *, struct MsgPort *))
			old_ppc_func_GT_GetIMsg) ( Self, iport);
}

void ppc_func_GT_ReplyIMsg (struct GadToolsIFace *Self, struct IntuiMessage *msg)
{
	if (reply_port == msg -> ExecMessage.mn_ReplyPort)
	{
		FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
		ReplyMsg( msg );
	}
	else
	{
		((void (*)( struct GadToolsIFace *, struct IntuiMessage *))
				old_ppc_func_GT_ReplyIMsg) ( Self, msg);
	}
}


