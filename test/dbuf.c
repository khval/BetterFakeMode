

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>


#include "init.h"


/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

	#ifndef M_PI
	#define M_PI 3.14159265358979323846
	#endif

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

	long min (long a,long b) { return (a < b ? a : b); }

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/

void draw_frame (struct RastPort *rp,long w,long h,long angle)
{
	long cx = w / 2;
	long cy = h / 2;
	long ry = min(w,h) / 4;
	long rx = sin(angle * M_PI / 180.0) * (float)ry;
	long pen = 1;

	if (rx < 0)
	{
		rx = -rx;
		pen = 2;
	}

	SetAPen (rp,0);
	RectFill (rp,cx - ry,cy - ry,cx + ry,cy + ry);
	SetAPen (rp,pen);
	DrawEllipse (rp,cx,cy,rx,ry);
}

#include "init.h"

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
		320,
		200,
		3,
		0,1,
		(ULONG) NULL,
		CUSTOMSCREEN,
		&MyFont,
		"My Own Screen",
		NULL,
		NULL
	};

void set_colors( struct Screen *src )
{
	 SetRGB32( &src -> ViewPort, 0, 0x55555555,0x99999999,0xDDDDDDD );
	 SetRGB32( &src -> ViewPort, 1, 0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF );
	 SetRGB32( &src -> ViewPort, 2, 0x00000000,0x00000000,0x00000000 );
	 SetRGB32( &src -> ViewPort, 3, 0xFFFFFFFF,0x00000000,0x00000000 );
	 SetRGB32( &src -> ViewPort, 4, 0x00000000,0xFFFFFFFF,0x00000000 );
	 SetRGB32( &src -> ViewPort, 5, 0x00000000,0x00000000,0xFFFFFFFF );
}

void show_screenbuffer( struct ScreenBuffer *sb )
{
	if (! sb) return;

	Printf("sb_BitMap: %08lx\n", sb -> sb_BitMap);
	Printf("sb_DBufInfo: %08lx\n", sb -> sb_DBufInfo);
}


int main (void)
{
	struct Screen *scr;

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

//	struct Screen *scr = OpenScreenTags (NULL,SA_LikeWorkbench,TRUE,TAG_END);

	scr = OpenScreen( &myscr );

	if (scr)
	{
		struct ScreenBuffer *sbuff[2];
		struct MsgPort *dispport = CreateMsgPort();
		struct MsgPort *safeport = CreateMsgPort();

		sbuff[0] = AllocScreenBuffer (scr,NULL,SB_SCREEN_BITMAP);
		sbuff[1] = AllocScreenBuffer (scr,NULL,SB_COPY_BITMAP);

Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

		if (sbuff[0] && sbuff[1] && dispport && safeport)
		{
			long angle = 0;
			struct RastPort rp = {0};
			BOOL SafeToWrite = TRUE;
			BOOL SafeToChange = TRUE;
			long CurBuffer = 0;
			BOOL cont = TRUE;

			InitRastPort (&rp);

			show_screenbuffer( sbuff[0] );
			show_screenbuffer( sbuff[1] );

Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

			do
			{
				if (!SafeToWrite)
				{
					while (!GetMsg (safeport))
					{
						Printf ("wait safe\n");

						if (Wait ((1L << safeport->mp_SigBit) | SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
						{
							cont = FALSE;
							break;
						}
					}
					SafeToWrite = TRUE;
				}


Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

				if (cont)
				{
					Printf ("draw %ld\n",CurBuffer);
					angle = (angle + 1) % 360;
					rp.BitMap = sbuff[CurBuffer]->sb_BitMap;
					draw_frame (&rp,scr->Width,scr->Height,angle);

					if (!SafeToChange)
					{
						while (!GetMsg (dispport))
						{
							Printf ("wait disp\n");

							if (Wait ((1L << dispport->mp_SigBit) | SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
							{
								cont = FALSE;
								break;
							}
						}
						SafeToChange = TRUE;
					}
				}

Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

				if (cont)
				{
					WaitBlit();
					sbuff[CurBuffer]->sb_DBufInfo->dbi_SafeMessage.mn_ReplyPort = safeport;
					sbuff[CurBuffer]->sb_DBufInfo->dbi_DispMessage.mn_ReplyPort = dispport;
					Printf ("display %ld\n",CurBuffer);

Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

					while (!ChangeScreenBuffer (scr,sbuff[CurBuffer]))
					{
						Printf ("change failed\n");
						Delay (TICKS_PER_SECOND / 10);

						if (SetSignal (0,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
						{
							cont = FALSE;
							break;
						}
					}

Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

					SafeToChange = FALSE;
					SafeToWrite  = FALSE;
					CurBuffer ^= 1;

					if (SetSignal (0,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
						cont = FALSE;
				}
			}
			while (cont);


			Printf ("*** Break\n");

			if (!SafeToWrite)
			{
				while (!GetMsg (safeport))
				{
					Printf ("wait safe\n");

					if (Wait ((1L << safeport->mp_SigBit) | SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
					{
						Printf ("*** Break\n");
						break;
					}
				}
			}

Printf("%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

			if (!SafeToChange)
			{
				while (!GetMsg (dispport))
				{
					Printf ("wait disp\n");

					if (Wait ((1L << dispport->mp_SigBit) | SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C)
					{
						Printf ("*** Break\n");
						break;
					}
				}
			}
		}

		if (sbuff[0])
		{
			FreeScreenBuffer (scr,sbuff[0]);
		}
		else printf("buffer 0 not allocated\n");

		if (sbuff[1])
		{
			FreeScreenBuffer (scr,sbuff[1]);
		}
		else printf("buffer 1 not allocated\n");

		if (safeport) DeleteMsgPort (safeport);
		if (dispport) DeleteMsgPort (dispport);

		CloseScreen (scr);
	}
	else
	{
		printf("can't cant OpenScreen\n");
	}

	close_libs();

	return (0);
}

/*----------------------------------------------------------------------------*/
/*                                                                            */
/*----------------------------------------------------------------------------*/
