
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "helper/screen.h"

struct Screen screens[max_screens];
bool allocatedScreen[max_screens];

int alloc_screen_in_list()
{
	int i = 0;
	for ( i = 0; i <  max_screens ; i++ )
	{
		if ( allocatedScreen[i] == false )
		{
			allocatedScreen[i] = true;
			return i;
		}
	}
	return -1;
}

void remove_screen_from_list(struct Screen *screen)
{
	int i = 0;
	struct Screen *thisScreen;

	for ( thisScreen = screens; thisScreen < screens + max_screens ; thisScreen++,i++ )
	{
		if (thisScreen == screen) 
		{
			allocatedScreen[i] = false;
		}
	}
}


bool is_fake_screen( struct Screen *screen )
{
	if ((screen >= screens) && (screen <= screens + max_screens)) return true;
	return false;
}

