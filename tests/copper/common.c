
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include "common.h"

struct Screen *screen=NULL;
struct Window *window=NULL;
struct ViewPort *viewport = NULL;
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
	if (window)
	{
		struct IntuiMessage *msg;

		ModifyIDCMP( window, 0L );  /* tell Intuition to stop sending more messages */

		while (msg = (struct IntuiMessage *) GetMsg( window -> UserPort ))
		{
			ReplyMsg( (struct Message *) msg);
		}

		if (viewport -> UCopIns)
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

void Box(struct RastPort *rp, int x0,int y0, int x1,int y1, int color)
{
	SetAPen( rp, color );
	Move( rp,x0,y0);
	Draw( rp,x1,y0);
	Draw( rp,x1,y1);
	Draw( rp,x0,y1);
	Draw( rp,x0,y0);
}

