

#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "hooks/intuition.h"
#include "helper/screen.h"


VOID ppc_func_RefreshGList (struct IntuitionIFace *Self, struct Gadget * gadgets, struct Window * window, struct Requester * requester, WORD numGad)
{
	if (is_fake_screen( window -> WScreen ))
	{

	}
	else
	{
		((void (*)( struct GadToolsIFace *, struct Gadget *, struct Window *, struct Requester *, WORD))
				old_ppc_func_RefreshGList) ( Self, gadgets, window, requester, numGad );
	}
}

UWORD ppc_func_AddGList(struct IntuitionIFace *Self, struct Window * window, struct Gadget * gadget, UWORD position, WORD numGad, struct Requester * requester)
{
}


