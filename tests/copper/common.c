
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include "common.h"

struct Screen *screen=NULL;
struct Window *window=NULL;
struct ViewPort *viewport=NULL;
PTR myucoplist=NULL;

void WaitLeftMouse(struct Window *window)
{
			BOOL running = TRUE;
			struct IntuiMessage *msg;
			ULONG class, code;
			ULONG win_mask = 1 << window -> UserPort ->mp_SigBit ;
	
		 	do
			{
				ULONG sig = Wait( win_mask | SIGBREAKF_CTRL_C);

				do
				{
					msg = (struct IntuiMessage *) GetMsg( window -> UserPort );

					if (msg)
					{
						code = msg -> Code;
						class = msg -> Class;
						switch (class)
						{
							case IDCMP_MOUSEBUTTONS :
								running = false;
								break;
						}
						ReplyMsg( (struct Message *) msg);
					}
				} while (msg);

			} while (running);
}

void closeDown()
{
	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

	if (window)
	{
		struct IntuiMessage *msg;

		while (msg = (struct IntuiMessage *) GetMsg( window -> UserPort ))
		{
			ReplyMsg( (struct Message *) msg);
		}

		if (viewport -> UCopIns)
		{
			FreeVPortCopLists(viewport);
			RemakeDisplay();
			myucoplist = NULL; // prevent myucoplist from being freed twice.
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

void Box(struct RastPort *rp, int x0,int y0, int x1,int y1, int color)
{
	SetAPen( rp, color );
	Move( rp,x0,y0);
	Draw( rp,x1,y0);
	Draw( rp,x1,y1);
	Draw( rp,x0,y1);
	Draw( rp,x0,y0);
}

void dumpUCopList( struct UCopList *copper )
{
	Printf("struct UCopList\n{\n");

   	Printf("	struct UCopList *Next: %08lx;\n", copper -> Next);
	Printf("	struct CopList  *FirstCopList: %08lx;\n", copper -> FirstCopList);
	Printf("	struct CopList  *CopList: %08lx;\n", copper -> CopList);
	Printf("}\n");
};


void dumpCopList( struct CopList *cl )
{
	Printf("struct CopList\n");
	Printf("{\n");
	Printf("    struct CopList  *Next: %08lx\n", cl -> Next);
	Printf("    struct CopList  *_CopList: %08lx\n", cl -> _CopList);
	Printf("    struct ViewPort *_ViewPort: %08lx\n", cl -> _ViewPort);
	Printf("    struct CopIns   *CopIns: %08lx\n", cl -> CopIns );
	Printf("    struct CopIns   *CopPtr: %08lx\n", cl -> CopPtr);
	Printf("    UWORD           *CopLStart: %08lx\n", cl -> CopLStart);
	Printf("    UWORD           *CopSStart: %08lx\n", cl -> CopSStart);
	Printf("    WORD             Count: %08lx\n", cl -> Count);
	Printf("    WORD             MaxCount: %08lx\n", cl -> MaxCount);
	Printf("    WORD             DyOffset: %08lx\n", cl -> DyOffset);
/*
	Printf("#ifdef V1_3\n");
	Printf("    UWORD           *Cop2Start: %08lx\n", cl -> Cop2Start);
	Printf("    UWORD           *Cop3Start: %08lx\n", cl -> Cop3Start);
	Printf("    UWORD           *Cop4Start: %08lx\n", cl -> Cop4Start);
	Printf("    UWORD           *Cop5Start: %08lx\n", cl -> Cop5Start);
	Printf("#endif\n");
*/
	Printf("    UWORD            SLRepeat: %08lx\n", cl -> SLRepeat);
	Printf("    UWORD            Flags: %08lx\n", cl -> Flags);
	Printf("};\n");
}

void dumpCopIns( struct CopIns *c, int cnt )
{
	while (cnt --)
	{
		Printf("%08lx: OpCode: %04lx, nxtlist: %08lx, Data: %04lx, %04lx\n", 
			c, 
			c -> OpCode,
			c -> u3.nxtlist,
			c -> u3.u4.u1.VWaitPos,
			c -> u3.u4.u2.HWaitPos
		);
		c++;
	}
}

