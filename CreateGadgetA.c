
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "common.h"
#include "hooks/gadtools.h"

void set_ng_gadget(struct Gadget *g ,const struct NewGadget *ng )
{
	g -> LeftEdge = ng -> ng_LeftEdge;
	g -> TopEdge = ng -> ng_TopEdge ;
	g -> Width = ng -> ng_Width ;
	g -> Height = ng -> ng_Height ;
	g -> GadgetID = ng -> ng_GadgetID ;
//	g -> VisualInfo = ng -> ng_VisualInfo ;
//	g -> TextAttr = ng -> ng_TextAttr ;
}

struct Gadget * fake_CreateGadgetA(ULONG kind,struct Gadget * previous,const struct NewGadget * ng,const struct TagItem * taglist)
{
	struct Gadget *g = new_struct( Gadget );

	if (g)
	{
		if (ng) set_ng_gadget(g ,ng);

		g -> NextGadget = NULL;

		if (previous) previous -> NextGadget = g;

	}
	return g;
}

