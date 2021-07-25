// OSCopper.e
// Native graphics example using OS friendly copperlist
		
#include <graphics/gfxbase>
#include <graphics/gfxmacros>
#include <graphics/copper>
#include <graphics/view>
#include <graphics/gfx>
#include <graphics/rastport>
#include <hardware/custom>
#include <intuition/intuition>
#include <intuition/screens>
#include <exec/memory>

bool initScreen()
{
	screen=OpenScreenTags(NULL,
			SA_TITLE,'OS Copper',
			SA_PENS,[-1]:INT,
			SA_DEPTH,4,
			SA_WIDTH, 320,
			SA_HEIGHT, 256,
			TAG_END);

	if (!screen) return false;

	window=OpenWindowTags(NULL,
			WA_IDCMP,IDCMP_MOUSEBUTTONS,
			WA_FLAGS,WFLG_NOCAREREFRESH |
					WFLG_ACTIVATE |
					WFLG_BORDERLESS |
					WFLG_BACKDROP,
			WA_CUSTOMSCREEN,screen,
			TAG_END);

	if (!window) return false;

	myucoplist=AllocMem(SIZEOF ucoplist,MEMF_PUBLIC OR MEMF_CLEAR);

	if (!myucoplist) return false;	

	return true;
}

void closeDown()
{
	if (window)
	{
		if (viewport.ucopins)
		{
			FreeVPortCopLists(viewport);
			RemakeDisplay();
		}
		CloseWindow(window);
		window = NULL;
	}

	if (screen)
	{
		CloseScreen(screen);
		screen = NULL;	
	}

	if (myucoplist)
	{
		FreeVec(myucoplist);
		myucoplist = NULL;

	}
}

void errors()
{
	if (!screen) PrintF('Unable to open screen.\n');
	if (!window) PrintF('Unable to open window.\n');
	if (!myucoplist) PrintF('Unable to allocate myucoplist memory.\n');
}

int main()
{

	if (initScreen())
	{

		int x,y
		int linestart=screen.barheight+1
		int lines=screen.height-linestart
		int width=screen.width
	
		struct ViewPort *viewport=ViewPortAddress(window);
		UInt32 backrgb=Int(viewport.colormap.colortable)
		struct RastPort *rport=window.rport;
		struct BitMap *bitmap=screen.rastport.bitmap;
		UInt32 modulo=bitmap.bytesperrow-40;
		UInt32 planesize=modulo*screen.height;
		UInt32 bitplane=bitmap.planes[0];
	
		SetStdRast(rport);
		SetColour(screen,0,0,0,0);
		SetColour(screen,1,255,255,255);
		SetRast(rport,1);
		Box(0,linestart,width-1,screen.height-1);
	
		for (y=0;y<64;y+=64)
			for (x=0;x<256;x+=64)
			{
				RectFill(rport,x,y,x+31,y+31)
				Box(x,y,32,32,1)
			}
			
			for (x=32;x<288;x+=64)
			{
				RectFill(rport,x,y+32,x+31,y+63)
				Box(x,y+32,32,32,1)
			}
		}
		
		CINIT(myucoplist,lines*4);
		CMOVEA(myucoplist,COLOR+2,$FFF);
		for (i=linestart;i<lines;i++)
		{
			CWAIT(myucoplist,i,0);
		
			if (i==127) CMOVEA(myucoplist,BPL1MOD,-1*(planesize/2));
			if (i==128) CMOVEA(myucoplist,BPL1MOD,modulo);
			if (i==127)
			{
				CMOVEA(myucoplist,BPLPT,Shr(bitplane,16));
			    CMOVEA(myucoplist,BPLPT+2,bitplane AND $FFFF);
			}
		
			CMOVEA(myucoplist,BPLCON3,0);
			CMOVEA(myucoplist,COLOR+2,(i-linestart) AND $FFF);
			CMOVEA(myucoplist,BPLCON3,$200);
			CMOVEA(myucoplist,COLOR+2,($FFF-i) AND $FFF);
		}
		CWAIT(myucoplist,i,0);
		CMOVEA(myucoplist,COLOR+2,backrgb);
		CEND(myucoplist);
	
		Forbid();
		viewport.ucopins=myucoplist;
		Permit();
		RethinkDisplay();
	
		WaitLeftMouse(window)
	
		if (window)
		{
			if (viewport.ucopins)
			{
				FreeVPortCopLists(viewport);
				RemakeDisplay();
			}
			CloseWindow(window);
		}
	}
	else
	{
		errors();		
	}

	closeDown();

	return 0;
}
