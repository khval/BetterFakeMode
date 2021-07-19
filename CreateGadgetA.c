
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

	if (g -> GadgetText)
	{
		g -> GadgetText -> IText = ng -> ng_GadgetText;
	}

	g -> NextGadget = NULL;
}

void set_gadget_tags( struct Gadget *g,const struct TagItem * tagList )
{
	const struct TagItem * tag;

	for (tag = tagList; (tag -> ti_Tag != TAG_DONE) || (tag -> ti_Tag != TAG_END); tag++)
	{
		switch ( tag -> ti_Tag )
		{
//			case 

		} // tag -> ti_Data );
	}
}

struct Gadget * fake_CreateGadgetA(ULONG kind,struct Gadget * previous,const struct NewGadget * ng,const struct TagItem * taglist)
{
	struct Gadget *g = new_struct( Gadget );

	if (g)
	{
		g -> Activation = 1;
		g -> Flags = GFLG_GADGHNONE;
		g -> MutualExclude = 0xFA8EFA8E;
		g -> GadgetText = new_struct( IntuiText );

		if (ng) set_ng_gadget(g ,ng);

		if (taglist) set_gadget_tags( g, taglist );

		g -> NextGadget = NULL;

		if (previous) previous -> NextGadget = g;

	}
	return g;
}

