
#include <stdbool.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/Layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

#include "helper/screen.h"

struct Screen screens[max_screens];
struct Layer_Info *LayerInfos[max_screens];
bool allocatedScreen[max_screens];

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

struct Screen *first_fake_screen()
{
	int n;
	for (n=0;n<max_screens;n++)
	{
		if (allocatedScreen[n]) return screens+n;
	}
	return NULL;
}

bool is_leagcy_mode( ULONG id )
{
	FPrintf( output, "MODE ID: %08lx\n",id);

	switch (id)
	{
		case 0x21000:	// LowRes
		case 0x29000: // HighRes
		case 0x29020: // SuperHighRes
		case 0x21004: // LowResLaced
		case 0x29004: // HighResLaced
		case 0x29024: // SuperHighResLaced
		case 0x29404: // HighResLaced DPF
			return true;
	}

	return false;
}

bool legacy_in_tags( const struct TagItem * tagList, bool legacy_status_maybe )
{
	bool has_mode_id = false;
	bool legacy_status = false;
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
				if (is_leagcy_mode( tag -> ti_Data )) legacy_status = true;
				break;
		}
	}

	if (has_mode_id == false)	// if we have no mode id, we most guess.
	{
		if (legacy_status_maybe) legacy_status=true;
	}

	return legacy_status;
}

