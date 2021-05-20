
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"

extern APTR video_mutex;

extern void RenderWindow(struct Window *win);

void fake_SetWindowTitles( struct Window *win, const char *winStr, const char *srcStr )
{
FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

FPrintf( output,"Title: %08lx, ScreenTitle: %08lx\n", win -> Title, win -> ScreenTitle);

	if (winStr)
	{
		if (win -> Title) free(win -> Title);
		win -> Title = strdup(  winStr );
	}

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	if (srcStr)
	{
		if (win -> ScreenTitle) free(win -> ScreenTitle);
		win -> ScreenTitle = strdup(  srcStr );
	}

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);

	RenderWindow(win);

FPrintf( output,"%s:%s:%ld\n",__FILE__,__FUNCTION__,__LINE__);
}

