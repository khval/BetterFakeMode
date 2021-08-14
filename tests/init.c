
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <exec/emulation.h>
#include <proto/libblitter.h>


#include "../common.h"
#include "../init.h"

struct Library 			*LibBlitterBase = NULL;
struct LibBlitterIFace		*ILibBlitter = NULL;

struct Library			*IntuitionBase = NULL;
struct IntuitionIFace		*IIntuition = NULL;

struct Library			*GraphicsBase = NULL;
struct GraphicsIFace		*IGraphics = NULL;

struct Library 			 *AslBase = NULL;
struct AslIFace 			 *IAsl = NULL;


BOOL open_lib( const char *name, int ver , const char *iname, int iver, struct Library **base, struct Interface **interface)
{
	*interface = NULL;
	*base = OpenLibrary( name , ver);

	if (*base)
	{
		 *interface = GetInterface( *base,  iname , iver, TAG_END );
		if (!*interface) Printf("Unable to getInterface %s for %s %ld!\n",iname,name,ver);
	}
	else
	{
	   	Printf("Unable to open the %s %ld!\n",name,ver);
	}
	return (*interface) ? TRUE : FALSE;
}

void close_lib_all( struct Library **Base, struct Interface **I )
{
	if (*Base) CloseLibrary(*Base); *Base = 0;
	if (*I) DropInterface((struct Interface*) *I); *I = 0;
}

bool open_libs()
{
	if ( ! open_lib( "intuition.library", 51L , "main", 1, &IntuitionBase, (struct Interface **) &IIntuition  ) ) return FALSE;
	if ( ! open_lib( "graphics.library", 54L , "main", 1, &GraphicsBase, (struct Interface **) &IGraphics  ) ) return FALSE;
	if ( ! open_lib( "libblitter.library", 53L , "main", 1, &LibBlitterBase, (struct Interface **) &ILibBlitter  ) ) return FALSE;
	if ( ! open_lib( "asl.library", 53L , "main", 1, &AslBase, (struct Interface **) &IAsl  ) ) return FALSE;

	return TRUE;
}

void close_libs()
{
	close_lib(Intuition);
	close_lib(Graphics);
	close_lib(LibBlitter);
	close_lib(Asl);
}

