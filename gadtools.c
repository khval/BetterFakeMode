#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/layers.h>

#include "common.h"
#include "helper/screen.h"

void addWinGadget(struct Window *win, ULONG icon_s, ULONG flags)
{
	struct Gadget *g = new_struct( Gadget );

	if (g)
	{
		g -> Flags = flags;
		g -> NextGadget = win -> FirstGadget;
		g -> Width = icon_s;
		g -> Height  = icon_s;
		g -> GadgetID = 0;
		win -> FirstGadget = g;
	}
}

struct Gadget *lastGadget( struct Window *win, ULONG *index)
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);

	struct Gadget *g;
	struct Gadget *lastGadget = NULL;
	*index = 0;
	for (g = win -> FirstGadget; g;g = g -> NextGadget)	
	{
		lastGadget = g;
		*index ++;
	}

	FPrintf( output, "%s:%ld - Last Gadget is %ld\n",__FUNCTION__,__LINE__, *index);
	return lastGadget;
}

struct Gadget *dupGadgat(struct Gadget *g)
{
	struct Gadget *new_g = new_struct( Gadget );
	if (new_g)	*new_g = *g;
	new_g -> NextGadget = NULL;
	return new_g;
}

ULONG attachGadget(struct Gadget *g, struct Window *win)
{
	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);
	ULONG index =0;
	if (g)
	{
		struct Gadget *last_gadget = lastGadget( win, &index );

		if (last_gadget) 
		{
			last_gadget -> NextGadget = g;
		}
		else win -> FirstGadget = g ;
	}
	return index;
}

void freeGadget( struct Gadget *g )
{
	FreeVec( g );
}

void freeWinGadgets( struct Window *win )
{
	struct Gadget *next_g;
	struct Gadget *g = win -> FirstGadget;

	FPrintf( output, "%s:%ld\n",__FUNCTION__,__LINE__);

	while (g)
	{
		next_g = g -> NextGadget;
		freeGadget(g);
		g = next_g;
	}
	
	win -> FirstGadget = NULL;
}

