
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
		640,
		480,
		8,
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
	int n;

	for (n=0;n<256;n++)
	{
	 	SetRGB32( &src -> ViewPort, n, 0x01010101*n,n==0 ? 0x00000000 : 0xFFFFFFFF, 0x0000000 );
	}
}


void pplot( struct BitMap *bm, int x, int y)
{
	int bx = 7- (x&7) ;
	x /= 8;

	*(bm -> Planes[0] + x + y * bm -> BytesPerRow ) |= 1L<<bx;
}

struct TagItem srctags[]=
{
	{SA_DisplayID, 0x00021000},
	{SA_Width, 0x00000280},
	{SA_Height, 0x000003C0},
	{SA_Depth, 0x00000008},
	{SA_Quiet, 0x00000001},
	{SA_Type, 0x0000000F},
//	{SA_Behind, 0x00000001},
	{SA_AutoScroll, 0x00000000},
	{TAG_END, 0x00000000}
};

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

	src = OpenScreenTagList( &myscr, &srctags );

	if (src)
	{
		int n,x,y;
		ULONG Result;
		ULONG is_interleaved;
		struct VisualInfo *vi;

		Result = GetScreenAttr( src, SA_Interleaved, &is_interleaved, sizeof(ULONG) );

		vi = GetVisualInfo(src, TAG_DONE );

		set_colors( src );

		rp = &src -> RastPort;
		SetAPen(rp, 1);
		SetBPen(rp,2);

		for (n=0;n<256;n++)
		{
			x = n % 0x10;
			y = n / 0x10;

			SetAPen( rp, n);
			RectFill( rp , x*10,y*10,x*10+8,y*10+8);
		}

		printf("Press enter to quit\n");
		getchar();
		CloseScreen( src ) ;
	}

	close_libs();
}

