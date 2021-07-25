
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>
#include <proto/asl.h>


#include "init.h"

struct Library			*IntuitionBase;
struct IntuitionIFace	*IIntuition;

struct Library			*GraphicsBase;
struct GraphicsIFace	*IGraphics;

struct Library			*ASLBase;
struct ASLIFace		*IASL;

struct Library			*GadToolsBase;
struct GadToolsIFace	*IGadTools;

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
	if ( ! open_lib( "asl.library", 51L , "main", 1, &ASLBase, (struct Interface **) &IASL ) ) return FALSE;
	if ( ! open_lib( "gadtools.library", 51L , "main", 1, &GadToolsBase, (struct Interface **) &IGadTools  ) ) return FALSE;
	if ( ! open_lib( "graphics.library", 54L , "main", 1, &GraphicsBase, (struct Interface **) &IGraphics  ) ) return FALSE;
	if ( ! open_lib( "intuition.library", 53L , "main", 1, &IntuitionBase, (struct Interface **) &IIntuition  ) ) return FALSE;

	return TRUE;
}


void close_libs()
{
	close_lib(ASL);
	close_lib(GadTools);
	close_lib(Intuition);
	close_lib(Graphics);
}

