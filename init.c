
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"

struct Library			*LayersBase = NULL;
struct LayersIFace		*ILayers = NULL;

struct Library			*IntuitionBase = NULL;
struct IntuitionIFace		*IIntuition = NULL;

struct Library			*GraphicsBase = NULL;
struct GraphicsIFace		*IGraphics = NULL;

APTR video_mutex = NULL;

#define lpage 256*8

unsigned char bits2bytes_data[256*8*8];		// 256 values, 8pixels / bits, 8 plaines
unsigned char *bits2bytes[256*8];			// 256 values. 

unsigned char **bits2bytes0 = bits2bytes + 256*0;
unsigned char **bits2bytes1 = bits2bytes + 256*1;
unsigned char **bits2bytes2 = bits2bytes + 256*2;
unsigned char **bits2bytes3 = bits2bytes + 256*3;
unsigned char **bits2bytes4 = bits2bytes + 256*4;
unsigned char **bits2bytes5 = bits2bytes + 256*5;
unsigned char **bits2bytes6 = bits2bytes + 256*6;
unsigned char **bits2bytes7 = bits2bytes + 256*7;



void initBits2Bytes()
{
	int p,n,b;
	unsigned char *page;
	unsigned char *at;

	for (p=0;p<8;p++)
	{
		page = bits2bytes_data + lpage * p;

		for (n=0;n<256;n++) 
		{
			at= page + n*8;
			for (b=0; b<8;b++) at[7-b] = n & 1L<<b ? 1L<<p: 0;		// we revere the bits.
			bits2bytes[256*p+n] = at;
		}
	}
}

/*
void dump_num( int n, int p )
{
	unsigned char *c;
	unsigned char *at ;

	at = bits2bytes[256*p +n] ;

	printf("%02x: ",n);
	for (c = at; c < at +8 ; c++) printf("%02X",*c);

	printf("\n");

}

void dump_page( int p )
{
	int n;
	for (n=0;n<256;n++) dump_num(  n,  p );
}
*/

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


bool open_libs()
{
	if ( ! open_lib( "layers.library", 51L , "main", 1, &LayersBase, (struct Interface **) &ILayers ) ) return FALSE;
	if ( ! open_lib( "intuition.library", 51L , "main", 1, &IntuitionBase, (struct Interface **) &IIntuition  ) ) return FALSE;
	if ( ! open_lib( "graphics.library", 54L , "main", 1, &GraphicsBase, (struct Interface **) &IGraphics  ) ) return FALSE;

	video_mutex = (APTR) AllocSysObjectTags(ASOT_MUTEX, TAG_DONE);
	if ( ! video_mutex) return FALSE;

	initBits2Bytes();
//	dump_page( 2 );

	return TRUE;
}

void close_libs()
{
	if (video_mutex) 
	{
		FreeSysObject(ASOT_MUTEX, video_mutex); 
		video_mutex = NULL;
	}

	if (LayersBase) CloseLibrary(LayersBase); LayersBase = 0;
	if (ILayers) DropInterface((struct Interface*) ILayers); ILayers = 0;

	if (IntuitionBase) CloseLibrary(IntuitionBase); IntuitionBase = 0;
	if (IIntuition) DropInterface((struct Interface*) IIntuition); IIntuition = 0;

	if (GraphicsBase) CloseLibrary(GraphicsBase); GraphicsBase = 0;
	if (IGraphics) DropInterface((struct Interface*) IGraphics); IGraphics = 0;
}


