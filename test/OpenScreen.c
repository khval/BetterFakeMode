
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

struct NewScreen myscr = 
	{
		0,0,
		320,
		400,
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
	struct Screen *src;
	struct RastPort *rp;

	if (open_libs()==FALSE)
	{
		Printf("failed to open libs!\n");
		close_libs();
		return 0;
	}

	src = OpenScreen( &myscr );

	if (src)
	{
		int x,y;
		ULONG Result;
		ULONG is_interleaved;
		struct VisualInfo *vi;

		Result = GetScreenAttr( src, SA_Interleaved, &is_interleaved, sizeof(ULONG) );

		printf("Screen -> Flags: %08x\n", src->Flags );
		printf("is_interleaved: %s\n", is_interleaved ? "Yes" : "No");

		printf("src -> BarHeight: %d\n", src -> BarHeight );
		printf("src -> BarVBorder: %d\n", src -> BarVBorder );
		printf("src -> BarHBorder: %d\n", src -> BarHBorder );
		printf("src -> MenuVBorder: %d\n", src -> MenuVBorder );
		printf("src -> MenuHBorder: %d\n", src -> MenuHBorder );

		printf("src -> WBorTop: %d\n", src -> WBorTop );
		printf("src -> WBorLeft: %d\n", src -> WBorLeft );
		printf("src -> WBorRight: %d\n", src -> WBorRight );
		printf("src -> WBorBottom: %d\n", src -> WBorBottom );

		printf("src -> NextScreen: %08x\n", src -> NextScreen );
		printf("src -> FirstWindow: %08x\n", src -> FirstWindow );
		printf("src -> FirstGadget: %08x\n", src -> FirstGadget );
		printf("src -> BarLayer: %08x\n", src -> BarLayer );
		printf("src -> ExtData: %08x\n", src -> ExtData );
		printf("src -> UserData: %08x\n", src -> UserData );

		vi = GetVisualInfo(src, TAG_DONE );

		if (vi)
		{
			printf("found visual info, what is it?\n");
		}
		else
		{
			printf("BUG: no visual info\n");
		}

		set_colors( src );

		rp = &src -> RastPort;
		SetAPen(rp, 1);
		SetBPen(rp,2);


		for (y=0;y<50;y++)
		{
			for (x=0;x<50;x++)
			{
				pplot( src -> RastPort.BitMap , x,y);
				pplot( src -> RastPort.BitMap , y,x);

			}
			Delay(1);

		}

		Move(rp,50,50);
		Draw(rp,100,100);
		Move(rp,100,50);
		Draw(rp,50,100);

		Move(rp,50,20);
		Text(rp,"Amiga",5);


		printf("Press enter to quit\n");
		getchar();
		CloseScreen( src ) ;
	}

	close_libs();
}

