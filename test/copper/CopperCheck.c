// CopperCheck
// Native graphics example using OS friendly copperlist
// Displays checker pattern then scrolls each line indivdiually

#include	<graphics/gfxbase>
#include	<graphics/gfxmacros>
#include	<graphics/copper>
#include	<graphics/view>
#include	<hardware/custom>
#include	<intuition/intuition>
#include	<intuition/screens>
#include	<exec/memory>

typedef void * PTR;

struct Screen *screen=NULL;
struct Window *window=NULL;
PTR myucoplist=NULL;
struct ViewPort *viewport = NULL;
struct RastPort *rport;

int i;
ULONG linestart,lines,backrgb,
ULONG x,y,class;

bool initScreen()
{
	screen=OpenScreenTags(NIL,SA_QUIET,TRUE,
                SA_DEPTH,1,
                SA_WIDTH,320,
                SA_HEIGHT,256,
                SA_COLORS,[0,$FFF,0,0]:INT,
                SA_TYPE,CUSTOMSCREEN,
                TAG_END);

	if (!screen) return false;

	window=OpenWindowTags(NULL,
				WA_IDCMP,IDCMP_MOUSEBUTTONS.
                WA_FLAGS,WFLG_NOCAREREFRESH |
                        WFLG_ACTIVATE |
                        WFLG_BORDERLESS |
                        WFLG_BACKDROP |
                        WFLG_RMBTRAP,
                WA_CUSTOMSCREEN,screen,
                WA_LEFT,0,
                WA_TOP,0,
                WA_WIDTH,320,
                WA_HEIGHT,256,
                TAG_END);

	if (!window) return false;

	myucoplist=AllocVecTags(sizeof(ucoplist),MEMF_PUBLIC | MEMF_CLEAR)

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
		viewport=ViewPortAddress(window);
		rport=window.rport;
		backrgb=Int(viewport.colormap.colortable);
		SetStdRast(rport);
		SetColour(screen,0,0,0,0);
		SetColour(screen,1,255,255,255);
	
		for (y=0;y<256;y+=64) for (x=0;x<192;x+=64) Box(x,y,32,32,1);
	
		CINIT(myucoplist,lines*4)
	
		for (i=linestart;i<lines;i++)
		{
			CWAIT(myucoplist,i,0);
	  		CMOVEA(myucoplist,BPLCON3,0);
	  		CMOVEA(myucoplist,COLOR+2,(i-linestart) & 0xFFF);
	  		CMOVEA(myucoplist,BPLCON3,0x200);
	  		CMOVEA(myucoplist,COLOR+2,(0xFFF-i) & 0xFFF);
		}
	
		CWAIT(myucoplist,i,0);
		CMOVEA(myucoplist,COLOR+2,backrgb);
		CEND(myucoplist);
	
		Forbid();
	 	viewport.ucopins=myucoplist;
		Permit();
		RethinkDisplay();
	
	 	do
		{
			class=WaitIMessage(window);
		} until ((class==IDCMP_MOUSEBUTTONS) && (MsgCode()==MENUUP));
	}
	else
	{
		errors();		
	}

	closeDown();

	return 0;
}
