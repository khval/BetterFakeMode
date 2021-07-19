
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "common.h"

unsigned int		EventCode;
int				EventWindow;
struct Gadget		*GadgetHit;
struct Window		*current_win;


int event_mask = 0;

int __blitz_use_window = 0;


void init_blitz_windows()
{
	int 		i;

	for (i=0;i<max_windows;i++)
	{
		win[i] = NULL;
	}
}


int Window(int nr,int x,int y,int w,int h,int flags,char *name,int pen1,int pen2)
{
	__blitz_use_window = 0;

	if (win[nr]) return FALSE;

	flags |= WFLG_REPORTMOUSE ;

	win[nr] = OpenWindowTags (NULL,
			WA_Title,NULL,
			WA_Left,x,WA_Top,y,
			WA_Width,w,WA_Height,h,
			WA_Flags, flags,
/*
				WFLG_GIMMEZEROZERO|
				WFLG_ACTIVATE|
				WFLG_BORDERLESS,
*/
			WA_IDCMP,
			IDCMP_MOUSEMOVE |
			IDCMP_CLOSEWINDOW |
			IDCMP_MOUSEBUTTONS |
			IDCMP_GADGETUP | 
			IDCMP_GADGETDOWN, 

			TAG_END);

	if (!win[nr]) return FALSE;

	__blitz_use_window = nr;

	return TRUE;
}


	/* remove and reply all IntuiMessages on a port that
	 * have been sent to a particular window
	 * (note that we don't rely on the ln_Succ pointer
	 *  of a message after we have replied it)
	 */

strip_intui_messages(struct MsgPort * mp,struct Window * win )
{
	struct IntuiMessage *msg;
	struct Node *succ;

	msg = (struct IntuiMessage *) mp->mp_MsgList.lh_Head;

	while( succ =  msg->ExecMessage.mn_Node.ln_Succ ) 
	{

		if( msg->IDCMPWindow ==  win )
		{
			/*
			* Intuition is about to free this message.
			* Make sure that we have politely sent it back.
			*/

			Remove( (void *) msg );
			ReplyMsg( (void *) msg );
		}

		msg = (struct IntuiMessage *) succ;
	}
}

void BCloswindow(int nr)
{
	if (win[nr])
	{
		CloseWindow(win[nr]);
		win[nr]=NULL;
	}
}

void Uswindow(int win)
{
	__blitz_use_window = win;
}

int WindowY()
{
	if (!win[__blitz_use_window]) return -1;
	return win[__blitz_use_window] -> TopEdge;
}

int WindowX()
{
	if (!win[__blitz_use_window]) return -1;
	return win[__blitz_use_window]->LeftEdge;
}

int WindowWidth()
{
	if (!win[__blitz_use_window]) return -1;
	return win[__blitz_use_window]->Width;
}

int WindowHeight()
{
	if (!win[__blitz_use_window]) return -1;
	return win[__blitz_use_window]->Height;
}

void WLine(int x,int y,int x2,int y2, int color) 
{
	struct RastPort *rp;

	if (!win[__blitz_use_window]) return;

	rp = win[__blitz_use_window]->RPort;
	SetAPen(rp,color); 
	Move(rp,x,y); 
	Draw(rp,x2,y2);
}

void UseScreen() {}


void WBox( int x1,int y1,int x2,int y2, int color ) 
{  
	struct RastPort *rp;

	rp = win[__blitz_use_window]->RPort;
	SetAPen(rp,color); 

		RectFill( rp, x1, y1, x2, y2);
}

void BEnable() {}
void BDisable() {}
void Activate() {}
void Toggle() {}
void Redraw() {}
int GadgetStatus(int w,int nr) {}
void FlushEvents() {}

void WColour(int a, int b) 
{
	struct RastPort *rp;
	rp = win[__blitz_use_window]->RPort;
	SetAPen( rp, a );
	SetBPen( rp, b );
}

void UsePalette() {}
void MouseWait() {}
void LoadShapes() {}
int ShapeHeight() { return 30; }
int ShapeWidth() { return 30; }
void GadgetBorder() {}



LONG update_event_mask()
{
	int 		i;

	event_mask = 0;

	for (i=0;i<max_windows;i++)
	{
		Printf("win[%ld] is %08lx\n",i,win[i]);

		if (win[i]) event_mask |= 1 << win[i]-> UserPort->mp_SigBit ;
	}

	return event_mask;
}


int Event()
{
	int 			i;
	unsigned int	imsgClass;
	struct IntuiMessage *imsg = 0;

	EventCode = 0;
	GadgetHit = 0;
	current_win = 0;
	EventWindow = 0;

	for (i=0;i<max_windows;i++)
	{
		imsg = 0;
		imsgClass = 0;

		if (win[i])
		{
			if (imsg = GT_GetIMsg(win[i]->UserPort))
			{
				EventWindow	=	i;
			 	current_win	=	win[i]; 	
				imsgClass		=	imsg->Class;
				EventCode	=	imsg->Code;
				GadgetHit		=	(struct Gadget *) imsg -> IAddress;
				GT_ReplyIMsg(imsg);
			}

			switch (imsgClass)
			{
				case IDCMP_REFRESHWINDOW:

					GT_BeginRefresh(current_win);
					GT_EndRefresh(current_win, TRUE);
					break;

				default:	break;
			}
		}
		if (imsg) break;
	}
	return imsgClass;
}

static LONG _win_index = 0;

LONG _GetAEvent()
{
	struct Window *cwin;
	struct IntuiMessage *imsg = 0;
	ULONG	imsgClass = 0;
	ULONG count;

	EventCode = 0;
	GadgetHit = NULL;

	for (count=0;count<10;count++)	// try 10 times to find a event..
	{
		// give it a even chance of event being read.
		_win_index = (_win_index + 1) % max_windows;

		cwin = win[_win_index];	// get current window.
		if (cwin)
		{
			imsg = GT_GetIMsg(cwin -> UserPort);

			Printf("%08lx = GT_GetIMsg(cwin -> UserPort: %08lx)\n", imsg, cwin -> UserPort);

			if (imsg)
			{
				Printf("Success\n");
				current_win = cwin;
				imsgClass		=	imsg->Class;
				EventCode	=	imsg->Code;
				GadgetHit		=	(struct Gadget *) imsg -> IAddress;

				Printf("GT_ReplyIMsg(imsg: %08lx)\n", imsg);
				GT_ReplyIMsg(imsg);
				Printf("Success\n");
			}
			else
			{
				imsgClass = 0;
				EventCode = 0;
				GadgetHit	= NULL;
			}

			switch (imsgClass)
			{
				case IDCMP_REFRESHWINDOW:

					GT_BeginRefresh(cwin);
					GT_EndRefresh(cwin, TRUE);
					break;

				default:	break;
			}
		}

		if (imsgClass) return imsgClass;	// we found a event so exit...
	}
	return 0;
}

LONG  WaitEvent()
{
	ULONG	imsgClass;
	struct IntuiMessage *imsg = 0;

	EventCode = 0;
	GadgetHit = 0;
	current_win = 0;

	imsgClass = _GetAEvent();
	if (imsgClass) return imsgClass;

	event_mask = update_event_mask();

	if (event_mask == 0)
	{
		Printf("event_mask %08lx\n",event_mask);
		Printf("nothing to wait for\n");
		return IDCMP_CLOSEWINDOW;
	}

	Wait(event_mask);
	return _GetAEvent();
}


int Exists(char *file)
{
	BPTR	lock;
	int		ret;

	ret = FALSE;

	lock = Lock(file,SHARED_LOCK);
	if (lock)
	{
		ret = TRUE;
		UnLock(lock);
	}
	return ret;
}

int Instr( char *text,char *item )
{
	int	found;
	int	ilen;
	int	match = 0;
	int	i;

	ilen=strlen(item);

	found = -1;

	for(i=0;i<strlen(text);i++)
	{
		if ((text[i]==item[match]) && (match<ilen) )
		{ 
			if (found==0) found = i;

			match ++; 
			if (match ==  ilen)
			{
				return found;
			}
		}
		else
		{
			match = 0;
			found = 0;
		}
	}
	return -1;
}

int WCursX() {};
int WCursY() {};

int WLocate(int x,int y) 
{
	struct RastPort *rp;
	rp = win[__blitz_use_window] -> RPort;
	Move(rp,x,y); 
};
