

#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "hooks/intuition.h"
#include "helper/screen.h"

extern BPTR output;

struct Gadget *dupGadgat(struct Gadget *g);
ULONG attachGadget(struct Gadget *g, struct Window *win);
void RenderGadgets(struct RastPort *rp, struct Gadget *g);


VOID ppc_func_RefreshGList (struct IntuitionIFace *Self, struct Gadget * gadgets, struct Window * window, struct Requester * requester, WORD numGad)
{
	if (is_fake_screen( window -> WScreen ))
	{
		struct RastPort *rp = window -> RPort;
		ULONG tmp_DetailPen = window -> DetailPen;
		SetAPen(rp,3);
		RenderGadgets( window -> RPort , window -> FirstGadget);
		SetAPen(rp,tmp_DetailPen);
	}
	else
	{
		((void (*)( struct IntuitionIFace *, struct Gadget *, struct Window *, struct Requester *, WORD))
				old_ppc_func_RefreshGList) ( Self, gadgets, window, requester, numGad );
	}
}

UWORD ppc_func_AddGList(struct IntuitionIFace *Self, struct Window * window, struct Gadget * gadget, UWORD position, WORD numGad, struct Requester * requester)
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);

	if (is_fake_screen( window -> WScreen ))
	{
		ULONG ret = 0;
		struct Gadget *g = gadget ;
		while ((position)&&(g)) g=g->NextGadget;

		if (numGad == -1)
		{
			while (g) 
			{
				FPrintf( output, "%s:%ld Gadget %08lx\n",__FUNCTION__,__LINE__,g);

				ret = attachGadget(dupGadgat(g), window);
				g=g->NextGadget;
			}
		}
		else
		{
			while ((g)&&(numGad))
			{
				FPrintf( output, "%s:%ld Gadget %08x\n",__FUNCTION__,__LINE__,g);

				ret = attachGadget(dupGadgat(g), window);
				g=g->NextGadget;
				numGad--;
			}
		}
		return ret;
	}
	else
	{
		return ((UWORD (*)(struct IntuitionIFace *, struct Window * , struct Gadget * , UWORD , WORD , struct Requester * ))
				old_ppc_func_AddGList) ( Self, window, gadget, position,numGad,requester );
	}
}


VOID ppc_func_ScreenPosition(struct IntuitionIFace *Self, struct Screen * screen, ULONG flags, LONG x1, LONG y1, LONG x2, LONG y2)
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
	((VOID (*) (struct IntuitionIFace *, struct Screen * , ULONG , LONG , LONG , LONG , LONG )) old_ppc_func_ScreenPosition) (Self, screen, flags, x1,y1,x2,y2 );
}

VOID ppc_func_MoveScreen(struct IntuitionIFace *Self, struct Screen * screen, WORD dx, WORD dy)
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
	((VOID (*) (struct IntuitionIFace *, struct Screen * , WORD, WORD )) old_ppc_func_MoveScreen) (Self, screen, dx, dy );
}

