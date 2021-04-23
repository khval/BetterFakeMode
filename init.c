
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

struct Library			*IntuitionBase = NULL;
struct IntuitionIFace		*IIntuition = NULL;

struct Library			*GraphicsBase = NULL;
struct GraphicsIFace		*IGraphics = NULL;

#define Printf IDOS->Printf

BOOL open_lib( const char *name, int ver , const char *iname, int iver, struct Library **base, struct Interface **interface)
{
	*interface = NULL;
	*base = IExec -> OpenLibrary( name , ver);

	if (*base)
	{
		 *interface = IExec -> GetInterface( *base,  iname , iver, TAG_END );
		if (!*interface) Printf("Unable to getInterface %s for %s %ld!\n",iname,name,ver);
	}
	else
	{
	   	Printf("Unable to open the %s %ld!\n",name,ver);
	}
	return (*interface) ? TRUE : FALSE;
}


bool open_libs()
{
	if ( ! open_lib( "intuition.library", 51L , "main", 1, &IntuitionBase, (struct Interface **) &IIntuition  ) ) return FALSE;
	if ( ! open_lib( "graphics.library", 54L , "main", 1, &GraphicsBase, (struct Interface **) &IGraphics  ) ) return FALSE;

	return TRUE;
}

void close_libs()
{
	if (IntuitionBase) IExec -> CloseLibrary(IntuitionBase); IntuitionBase = 0;
	if (IIntuition) IExec -> DropInterface((struct Interface*) IIntuition); IIntuition = 0;

	if (GraphicsBase) IExec -> CloseLibrary(GraphicsBase); GraphicsBase = 0;
	if (IGraphics) IExec -> DropInterface((struct Interface*) IGraphics); IGraphics = 0;
}


