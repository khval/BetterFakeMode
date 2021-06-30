
#include <stdio.h>
#include <stdbool.h>
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

ULONG fake_GetBitMapAttr (  struct BitMap * bm, ULONG value)
{
	switch (value)
	{
		case BMA_WIDTH:
			return bm -> BytesPerRow*8;
		case BMA_HEIGHT:
			return bm -> Rows;
		case BMA_DEPTH:
			return bm -> Depth;
		case BMA_FLAGS:
			return bm -> Flags;
		case BMA_ISRTG:
			return 0;
		case BMA_BYTESPERPIXEL:
			return 0;
		case BMA_BITSPERPIXEL:
			return bm -> Depth;
		case BMA_PIXELFORMAT:
			return 0;
		case BMA_ACTUALWIDTH:
			return bm -> BytesPerRow*8;
		case BMA_BYTESPERROW:
			return bm -> BytesPerRow;
	}

	return 0;
}


