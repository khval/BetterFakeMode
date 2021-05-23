
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "common.h"
#include "helper/screen.h"


bool has_window( struct Screen *s, struct Window *w )
{
	struct Window *i;
	for (i = s -> FirstWindow; i ; i = i -> NextWindow)
	{
		if (i == w) return true;
	}
	return false;
}

void no_block_ActivateWindow( struct Window *w )
{
	if (w != active_win)
	{
		struct Screen *src = w -> WScreen;
		struct Window *old_win = active_win ;

		active_win = w;
		if (has_window( src, old_win ))
		{
			RenderWindow(old_win);
		}
		RenderWindow(active_win);
	}
}

void fake_ActivateWindow( struct Window *w )
{
	MutexObtain(video_mutex);
	no_block_ActivateWindow(w);
	MutexRelease(video_mutex);
}


