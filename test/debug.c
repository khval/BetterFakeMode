
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>

void show_layer_Info(struct Layer_Info *li)
{
	printf(
		" top_layer: %08x\n"
		"resPtr1: %08x\n"
		"resPtr2: %08x\n"
		"FreeClipRects: %08x\n"
		"PrivateReserve3: %d\n"
		"ClipRectPool: %08x\n"
   	 	"Flags: %08x\n"
		"res_count: %d\n"
		"LockLayersCount: %d\n"
		"PrivateReserve5: %d\n"
		"BlankHook: %08x\n"
		"Extension: %08x\n",

			li -> top_layer,
			li -> resPtr1,
			li -> resPtr2,
			li -> FreeClipRects,
			li -> PrivateReserve3,
			li -> ClipRectPool,
   		 	li -> Flags,
			li -> res_count,
			li -> LockLayersCount,
			li -> PrivateReserve5,
			li -> BlankHook,
			li -> Extension);
}

void show_ArieaInfo(struct AreaInfo *ai)
{
	printf(
		"AreaInfo-> *VctrTbl: %08x\n"
    		"AreaInfo-> *VctrPtr: %08x\n"
		"AreaInfo-> *FlagTbl: %08x\n"
		"AreaInfo-> *FlagPtr: %08x\n"
		"AreaInfo-> Count: %d\n"
		"AreaInfo-> MaxCount: %d\n"
		"AreaInfo-> FirstX: %d\n"
		"AreaInfo-> FirstY: %d\n",
			ai->VctrTbl,
			ai->VctrPtr,
			ai->FlagTbl,
			ai->FlagPtr,
			ai->Count,
			ai->MaxCount,
			ai->FirstX,
			ai->FirstY);
}

void show_screen( struct Screen *src )
{
	printf("src -> FirstWindow: %08x\n", src -> FirstWindow);
	printf("src -> RastPort: %08x\n",&src -> RastPort);
	printf("src -> RastPort -> BitMap: %08x\n",src->RastPort.BitMap);

	printf("src -> ViewPort.DWidth: %d\n",src->ViewPort.DWidth);
	printf("src -> ViewPort.DHeight: %d\n",src->ViewPort.DHeight);
	printf("src -> ViewPort.DxOffset: %d\n",src->ViewPort.DxOffset);
	printf("src -> ViewPort.DyOffset: %d\n",src->ViewPort.DyOffset);
	printf("src -> ViewPort.Modes: %08x\n",src->ViewPort.Modes);
}

void show_rp(struct RastPort *rp)
{
	printf("win -> RastPort -> Layer: %08x\n", rp -> Layer);
	printf("win -> RastPort -> BitMap: %08x\n", rp -> BitMap);
	printf("win -> RastPort -> AreaPtrn: %08x\n", rp -> AreaPtrn);
	printf("win -> RastPort -> TmpRas: %08x\n", rp -> TmpRas);
	printf("win -> RastPort -> AreaInfo: %08x\n", rp -> AreaInfo);
	printf("win -> RastPort -> GelsInfo: %08x\n", rp -> GelsInfo);

	printf("win -> RastPort -> cp_x: %d\n", rp -> cp_x);
	printf("win -> RastPort -> cp_y: %d\n", rp -> cp_y);
	printf("win -> RastPort -> PenWidth: %d\n", rp -> PenWidth);
	printf("win -> RastPort -> PenHeight: %d\n", rp -> PenHeight);
}

void show_win( struct Window *win )
{
	printf("win -> LeftEdge %d\n",win->LeftEdge);
	printf("win -> TopEdge %d\n",win->TopEdge);
	printf("win -> Width: %d\n",win->Width);
	printf("win -> Height: %d\n",win->Height);
	printf("win -> GZZWidth: %d\n",win->GZZWidth);
	printf("win -> GZZHeight: %d\n",win->GZZHeight);
	printf("win -> RastPort: %08x\n",win->RPort);
	printf("win -> UserPort: %08x\n",win->UserPort);
	printf("win -> Flags: %08x\n",win->Flags);


	printf("win -> BorderTop: %d\n",win->BorderTop);
	printf("win -> BorderLeft: %d\n",win->BorderLeft);
	printf("win -> BorderRight: %d\n",win->BorderRight);
	printf("win -> BorderBottom: %d\n",win->BorderBottom);
}

void dump_gadget(struct Gadget *g)
{
	Printf("GadgetID: %04lx\n"
		"\t Activation: %ld\n"
		"\t LeftEdge: %ld\n"
		"\t TopEdge: %ld\n"
		"\t Width: %ld\n"
		"\t Height: %ld\n"
		"\t Flags: %04lx\n"
		"\t GadgetType: %04lx\n"
		"\t GadgetText: %s\n"
		"\t MutualExclude: %08lx\n"
		"\t SpecialInfo: %08lx\n"
		"\t UserData: %08lx\n"
		,
		g -> GadgetID,
		g -> Activation,
		g -> LeftEdge,
		g -> TopEdge,
		g -> Width,
		g -> Height,
		g -> Flags,
		g -> GadgetType,
		(g -> GadgetText ? g -> GadgetText -> IText : "NULL"),
		g -> MutualExclude,
		g -> SpecialInfo,
		g -> UserData
		);
}

void dump_window_gadgets(struct Window *win)
{
	Printf("dump_window_gadgets(Window: %08lx)\n",win);

	struct Gadget *g;
	for (g = win -> FirstGadget; g; g = g -> NextGadget)
	{
		dump_gadget(g);
	}
}
