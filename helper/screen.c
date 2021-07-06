
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/Layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "modeid.h"
#include "helper/screen.h"

struct Screen screens[max_screens];
struct Layer_Info *LayerInfos[max_screens];
bool allocatedScreen[max_screens];

#define GetFakeScreen(n) (screens + n)

extern BPTR output;

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

			if (LayerInfos[i])
			{
				DisposeLayerInfo(LayerInfos[i]);
				LayerInfos[i] = NULL;
			}
		}
	}
}


bool is_fake_screen( struct Screen *screen )
{
	if ((screen >= screens) && (screen <= screens + max_screens)) return true;
	return false;
}

int current_screen = 0;


void fake_screen_to_front(struct Screen *s)
{
	int n;
	for (n=0;n<max_screens;n++)
	{
		if (allocatedScreen[n])
		{
			if (GetFakeScreen(n)==s) 
			{
				current_screen = n;
				return;
			}
		}
	}
}

void fake_screen_to_back(struct Screen *s)
{
	int n,nn;
	nn = current_screen;

	for (n=0;n<max_screens;n++)
	{
		nn = (nn + 1) % max_screens;

		if (allocatedScreen[nn])
		{
			if ((GetFakeScreen(nn) != s) && ( allocatedScreen[nn]))
			{
				current_screen = nn;
				return;
			}
		}
	}
}

struct Screen *current_fake_screen()
{
	if (allocatedScreen[current_screen]) 
	{
		return GetFakeScreen(current_screen);
	}
	else
	{
		int n;
		for (n=0;n<max_screens;n++)
		{
			if (allocatedScreen[n])
			{
				current_screen = n;
				return GetFakeScreen(current_screen);
			}
		}
	}

	return NULL;
}

/*
struct Screen *first_fake_screen()
{
	int n;
	for (n=0;n<max_screens;n++)
	{
		if (allocatedScreen[n]) return screens+n;
	}
	return NULL;
}
*/

struct KownLgacyModes
{
	int w;
	int h;
};

struct KownLgacyModes LgacyModes[] =
{
	{320,200},	// NTSC, LOWRES
	{320,400},	// NTSC, LOWRES + interlaced
	{640,200},	// NTSC, HIRES
	{640,400},	// NTSC, HIRES + interlaced
	{320,256},	// PAL, LOWRES
	{320,512},	// PAL, LOWRES + interlaced
	{640,256},	// PAL, HIRES
	{640,512},	// PAL, HIRES + interlaced
	{-1,-1}
};

bool maybe_lagacy_mode(const struct NewScreen * newScreen)
{
	struct KownLgacyModes *lm;

	if (newScreen -> Depth<8) return true;
	if (newScreen -> Depth>8) return false;

	for (lm = LgacyModes; lm -> w != -1; lm++ )
	{
		if ((newScreen -> Width == lm -> w) && (newScreen -> Height == lm -> h)) return true;
	}

	return false;
}

extern struct modeT modes[];

struct modeT *is_leagcy_mode( ULONG id )
{
	struct modeT *mode;

	FPrintf( output, "MODE ID: %08lx\n",id);

	for (mode = modes; mode -> id ; mode++)
	{
		if (mode -> id == id) 
		{
			FPrintf( output, "MODE Name: %s\n",mode -> name);
			return mode;
		}
	}
	FPrintf( output, "MODE ID: %08lx -- not found\n",id);

	return NULL;
}

struct modeT *bestMode( ULONG monitor, ULONG w, ULONG h)
{
	ULONG _match; 
	ULONG match = ~0;
	struct modeT *mode;
	struct modeT *found = NULL;

	if (monitor)
	{
		for (mode = modes; mode -> id ; mode++)
		{
			if ( (mode -> id & MONITOR_ID_MASK) ==  monitor)
			{
				_match = abs(mode -> w-w) + abs(mode -> h-h);
				if (_match<match)
				{
					match = _match;
					found = mode;
				}
			}
		}
	}
	else
	{
		for (mode = modes; mode -> id ; mode++)
		{
			_match = abs(mode -> w-w) + abs(mode -> h-h);
			if (_match<match)
			{
				match = _match;
				found = mode;
			}
		}
	}

	return found;
}

struct modeT *legacy_in_tags( const struct TagItem * tagList, bool legacy_status_maybe )
{
	bool has_mode_id = false;
	struct modeT *legacy_mode = NULL;
	const struct TagItem *tag;

	if (tagList == NULL) return false;

	for (tag = tagList; tag -> ti_Tag != TAG_DONE; tag++)
	{
		switch (tag -> ti_Tag)
		{
			case SA_Depth:
				if (tag -> ti_Data < 9) legacy_status_maybe = true;
				FPrintf( output, "Depth: %ld\n",tag -> ti_Data);
				break;

			case SA_DisplayID:
				has_mode_id = true;
				legacy_mode = is_leagcy_mode( tag -> ti_Data );
				break;
		}
	}

	if (has_mode_id == false)	// if we have no mode id, we most guess.
	{
		if (legacy_status_maybe) legacy_mode=(struct modeT *) 0xFFFFFFFF;
	}

	return legacy_mode;
}

