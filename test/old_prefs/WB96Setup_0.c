
#include <stdio.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <string.h>

#include <exec/types.h>
#include <exec/ports.h>
#include <exec/interfaces.h>
#include <exec/libraries.h>
#include <dos/dos.h>
#include <intuition/intuition.h>
#include <intuition/classes.h>
#include <intuition/gadgetclass.h>

#include <utility/tagitem.h> 
#include <libraries/asl.h> 
#include <interfaces/asl.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include <proto/gadtools.h>
#include <proto/asl.h>
#include <proto/requester.h>
#include <proto/commodities.h>

#include "common.h"

#include <classes/requester.h>

struct Window		*win[max_windows];
struct Screen		*src;
struct VisualInfo *vi;

#include "gadtools_blitz.h"

//#include "../asl_blitz.i"

unsigned int DefaultIDCMP= 0x200|0x40|0x20;

char		*pa=NULL;
char		*fi;
short		WBW=0;
short		WBH=0;
char 	*pPar[]={"ENVARC:Excalibur","ENV:Excalibur"};
char		*prefs_name="ExcaliburSetup.prefs";


struct TextAttr MyFont =
{
	"topaz.font",
	TOPAZ_SIXTY,
	FS_NORMAL,
	FPF_ROMFONT
};

struct NewScreen myscr = 
	{
		0,0,
		640,
		480,
		2,
		0,1,
		(ULONG) NULL,
		CUSTOMSCREEN,
		&MyFont,
		"My Own Screen",
		NULL,
		NULL
	};

enum
{
	index_o_bar,
	index_o_time,
	index_o_mem,
	index_o_screens,
	index_i_stack,
	index_o_only_icons,
	index_o_open,
	index_o_close,
	index_o_show_icons,
	index_o_quit,
	index_o_auto_close,
	index_b_logo,
	index_t_logo,
	index_b_texture1,
	index_t_texture1,
	index_b_texture2,
	index_t_texture2,
	index_b_save,
	index_b_use,
	index_b_cancell,
	index_i_backcolor
};

void ChangeIcon();

//char	**delay_opt;

void dump_gadget(struct Gadget *g)
{
	Printf("id: %08lx LeftEdge: %ld, TopEdge: %ld, Width: %ld, Height: %ld, Flags: %08lx GadgetText: %s\n",
		g -> GadgetID,
		g -> LeftEdge,
		g -> TopEdge,
		g -> Width,
		g -> Height,
		g -> Flags,
		g -> GadgetText ? g -> GadgetText -> IText : "NULL");
}

#define rowy(n) ((wh*n)/rows)+2

int main()
{
	char	*pbak;
	char *a;
	char *c;
	char *e;
	int 	i;
	int 	ev;
	int	gh;
//	char	*tmp_str;
	int 	rows;
	int 	wh,ww;
	int	centerw;
	char	*newfile;
	struct Gadget *g;

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);Delay(5);

	if (open_libs()==FALSE)
	{
		close_libs();
		return 20;
	}

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);Delay(5);

	init_blitz_windows();

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);Delay(5);

	init_blitz_gedtools();

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);Delay(5);

#if use_custom_screen
	src = OpenScreen( &myscr );	
#else
	src = LockPubScreen(NULL);
	if (!src) return FALSE;
#endif


	vi = GetVisualInfo(src, TAG_DONE );


	Printf("%s:%ld\n",__FUNCTION__,__LINE__);Delay(5);

	if ((vi)&&(src))
	{
		win[0] = OpenWindowTags (NULL,
			WA_Title," Setup",
			WA_Left,src -> Width/2-162,
			WA_Top,src -> Height/2-100,
			WA_Width,320,
			WA_Height,300,
			WA_Flags,WFLG_CLOSEGADGET|WFLG_DRAGBAR|WFLG_DEPTHGADGET|WFLG_GIMMEZEROZERO|WFLG_ACTIVATE,
			WA_IDCMP,IDCMP_CLOSEWINDOW | IDCMP_GADGETUP | IDCMP_GADGETDOWN, 
			WA_CustomScreen, src,
			TAG_END)	;
	}
	else
	{
		if (!vi) printf("No VisualInfo\n");
		if (!src) printf("No Screen\n");
	}

	if (win[0])
	{
		int button_height;

#if use_gadtools
	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

		wh = win[0] -> GZZHeight ;
		ww = win[0] -> GZZWidth ;

		rows = 14;
		button_height = (wh / rows) - 2;

		centerw = win[0] -> GZZWidth /2 ;

	Printf("button_height %ld\n",button_height);

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

		GTButton(1,index_b_logo,		ww-90,		rowy(9),80,button_height,"Logo",0);
		GTButton(1,index_b_texture1,	ww-90,		rowy(10),80,button_height,"Texture1",0);
		GTButton(1,index_b_texture2,	ww-90,		rowy(11),80,button_height,"Texture2",0);
		GTButton(1,index_b_save,	10,			rowy(13),90,button_height,"Save",0);
		GTButton(1,index_b_use,		centerw-45,	rowy(13),90,button_height,"Use",0);
		GTButton(1,index_b_cancell,	ww-100,		rowy(13),90,button_height,"Cancell",0);

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);
/*
		if (GadgetHit = find_blitz_gadget(1,	index_o_open	))
		{
			GT_SetGadgetAttrs(GadgetHit,current_win,NULL,
				GA_Disabled, TRUE,
				TAG_DONE);
		}
*/
	Printf("%s:%ld\n",__FUNCTION__,__LINE__);
/*
		if (GadgetHit = find_blitz_gadget(1,	index_o_close	))
		{
			GT_SetGadgetAttrs(GadgetHit,current_win,NULL,
				GA_Disabled, TRUE,
				TAG_DONE);
		}
*/
	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

		AttachGTList(1,0);

		for (g = win[0] -> FirstGadget; g; g = g -> NextGadget)
		{
			dump_gadget(g);
		}

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

		do
		{
	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

			ev=WaitEvent();

			Printf("ev: %08lx\n",ev);

			switch (ev)
			{
				case IDCMP_GADGETUP:

					if (GadgetHit)
					{
						switch(GadgetHit -> GadgetID)
						{
							case index_b_cancell:	ev=IDCMP_CLOSEWINDOW;
									Printf("Pressed Cancel button\n");
									break;
						}
					}
					else
					{
						Printf("bad IDCMP_GADGETUP message\n");
					}
					break;
			}
			Delay(1);
		}
		while (ev!=IDCMP_CLOSEWINDOW);
#endif

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

		CloseWindow(win[0]);

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);

		FreeGTList(0);

		Printf("%s:%ld\n",__FUNCTION__,__LINE__);Delay(5);
	}

	if (vi)
	{
		FreeVisualInfo( vi );
		vi = NULL;
	}

	if (src)
	{
#if use_custom_screen
		CloseScreen( src ) ;
#else
 		UnlockPubScreen(NULL,src);
#endif
		src = NULL;
	}

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);Delay(5);

	close_libs();

	Printf("%s:%ld\n",__FUNCTION__,__LINE__);Delay(5);

	return 0;

}



