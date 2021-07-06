
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/gadtools.h>

#include "init.h"

struct TextAttr MyFont =
{
	"topaz.font",
	TOPAZ_SIXTY,
	FS_NORMAL,
	FPF_ROMFONT
};

struct NewScreen myscr1 = 
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

struct NewScreen myscr2 = 
	{
		0,0,
		640,
		480,
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


void pplot( struct BitMap *bm, int x, int y)
{
	int bx = 7- (x&7) ;
	x /= 8;

	*(bm -> Planes[0] + x + y * bm -> BytesPerRow ) |= 1L<<bx;
}

int main()
{
	int l,n;
	struct Screen *src[2];
	struct RastPort *rp;

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	src[0] = OpenScreen( &myscr1 );
	src[1] = OpenScreen( &myscr2 );

	if ((src[0])&&(src[1]))
	{
		int x,y;
		char tmp[1000];

		set_colors( src[0] );
		set_colors( src[1] );

		for (n=0;n<2;n++)
		{
			rp = &src[n] -> RastPort;
			SetAPen(rp, 1);
			SetBPen(rp,2);
			Move(rp,50,20);

			sprintf(tmp,"Screen %d",n);
			Text(rp,tmp,strlen(tmp));
		}

		for (l=0;l<10;l++)
		{
			for (n=0;n<2;n++)
			{
				ScreenToFront(src[n]);
				Delay(20);
			}
		}
	}

	for (n=0;n<2;n++)
	{
		if (src[n]) CloseScreen( src[n] ) ;
		src[n] = NULL;
	}

	close_libs();
}

