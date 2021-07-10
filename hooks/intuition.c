

#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "hooks/intuition.h"
#include "helper/screen.h"

extern BPTR output;

VOID ppc_func_RefreshGList (struct IntuitionIFace *Self, struct Gadget * gadgets, struct Window * window, struct Requester * requester, WORD numGad)
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);

	if (is_fake_screen( window -> WScreen ))
	{
		FPrintf( output, "%s:%ld - NYI\n",__FUNCTION__,__LINE__);
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
		FPrintf( output, "%s:%ld - NYI\n",__FUNCTION__,__LINE__);
	}
	else
	{
		return ((UWORD (*)(struct IntuitionIFace *, struct Window * , struct Gadget * , UWORD , WORD , struct Requester * ))
				old_ppc_func_AddGList) ( Self, window, gadget, position,numGad,requester );
	}
}


