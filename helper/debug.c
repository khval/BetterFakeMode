

#include <stdbool.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/layers.h>
#include <proto/intuition.h>
#include <proto/graphics.h>

#include <common.h>

struct debug_tag_info win_info_tags[]={
		{WA_Activate,"WA_Activate"},
		{WA_AlphaClips,"WA_AlphaClips"},
		{WA_AlphaHook,"WA_AlphaHook"},
		{WA_AmigaKey,"WA_AmigaKey"},
		{WA_AutoAdjust,"WA_AutoAdjust"},
		{WA_AutoAdjustDClip,"WA_AutoAdjustDClip"},
		{WA_Backdrop,"WA_Backdrop"},
		{WA_BackFill,"WA_BackFill"},
		{WA_BlockPen,"WA_BlockPen"},
		{WA_Borderless,"WA_Borderless"},
		{WA_BusyPointer,"WA_BusyPointer"},
		{WA_Checkmark,"WA_Checkmark"},
		{WA_CloseGadget,"WA_CloseGadget"},
		{WA_Colors,"WA_Colors"},
		{WA_ContextMenuHook,"WA_ContextMenuHook"},
		{WA_CustomScreen,"WA_CustomScreen"},
		{WA_DepthGadget,"WA_DepthGadget"},
		{WA_DetailPen,"WA_DetailPen"},
		{WA_DragBar,"WA_DragBar"},
		{WA_DropShadows,"WA_DropShadows"},
		{WA_FadeTime,"WA_FadeTime"},
		{WA_Flags,"WA_Flags"},
		{WA_Gadgets,"WA_Gadgets"},
		{WA_GimmeZeroZero,"WA_GimmeZeroZero"},
		{WA_GrabFocus,"WA_GrabFocus"},
		{WA_Height,"WA_Height"},
		{WA_HelpGroup,"WA_HelpGroup"},
		{WA_HelpGroupWindow,"WA_HelpGroupWindow"},
		{WA_Hidden,"WA_Hidden"},
		{WA_IDCMP,"WA_IDCMP"},
		{WA_InFrontOf,"WA_InFrontOf"},
		{WA_InnerHeight,"WA_InnerHeight"},
		{WA_InnerWidth,"WA_InnerWidth"},
		{WA_Left,"WA_Left"},
		{WA_MaxHeight,"WA_MaxHeight"},
		{WA_MaxWidth,"WA_MaxWidth"},
		{WA_MenuHelp,"WA_MenuHelp"},
		{WA_MenuHook,"WA_MenuHook"},
		{WA_MenuStrip,"WA_MenuStrip"},
		{WA_MinHeight,"WA_MinHeight"},
		{WA_MinWidth,"WA_MinWidth"},
		{WA_MouseLimits,"WA_MouseLimits"},
		{WA_MouseQueue,"WA_MouseQueue"},
		{WA_NewLookMenus,"WA_NewLookMenus"},
		{WA_NoCareRefresh,"WA_NoCareRefresh"},
		{WA_NoHitThreshold,"WA_NoHitThreshold"},
		{WA_NoMenuKeyVerify,"WA_NoMenuKeyVerify"},
		{WA_NotifyDepth,"WA_NotifyDepth"},
		{WA_Opaqueness,"WA_Opaqueness"},
		{WA_OverrideOpaqueness,"WA_OverrideOpaqueness"},
		{WA_Pointer,"WA_Pointer"},
		{WA_PointerDelay,"WA_PointerDelay"},
		{WA_PointerType,"WA_PointerType"},
		{WA_PubScreen,"WA_PubScreen"},
		{WA_PubScreenFallBack,"WA_PubScreenFallBack"},
		{WA_PubScreenName,"WA_PubScreenName"},
		{WA_ReportMouse,"WA_ReportMouse"},
		{WA_Reserved1,"WA_Reserved1"},
		{WA_Reserved2,"WA_Reserved2"},
		{WA_RMBTrap,"WA_RMBTrap"},
		{WA_RptQueue,"WA_RptQueue"},
		{WA_ScreenTitle,"WA_ScreenTitle"},
		{WA_ShapeHook,"WA_ShapeHook"},
		{WA_ShapeRegion,"WA_ShapeRegion"},
		{WA_SimpleRefresh,"WA_SimpleRefresh"},
		{WA_SizeBBottom,"WA_SizeBBottom"},
		{WA_SizeBRight,"WA_SizeBRight"},
		{WA_SizeGadget,"WA_SizeGadget"},
		{WA_SmartRefresh,"WA_SmartRefresh"},
		{WA_StayTop,"WA_StayTop"},
		{WA_SuperBitMap,"WA_SuperBitMap"},
		{WA_TabletMessages,"WA_TabletMessages"},
		{WA_Title,"WA_Title"},
		{WA_ToolBox,"WA_ToolBox"},
		{WA_Top,"WA_Top"},
		{WA_UserPort,"WA_UserPort"},
		{WA_WBenchWindow,"WA_WBenchWindow"},
		{WA_Width,"WA_Width"},
		{WA_WindowBox,"WA_WindowBox"},
		{WA_WindowName,"WA_WindowName"},
		{WA_Zoom,"WA_Zoom"},
		{0,NULL}};

struct debug_tag_info screen_info_tags[]={
		{SA_ActiveWindow,"SA_ActiveWindow"},
		{SA_AutoScroll,"SA_AutoScroll"},
		{SA_BackChild,"SA_BackChild"},
		{SA_BackFill,"SA_BackFill"},
		{SA_Behind,"SA_Behind"},
		{SA_BitMap,"SA_BitMap"},
		{SA_BlockPen,"SA_BlockPen"},
		{SA_ColorMapEntries,"SA_ColorMapEntries"},
		{SA_Colors,"SA_Colors"},
		{SA_Colors32,"SA_Colors32"},
		{SA_Compositing,"SA_Compositing"},
		{SA_DClip,"SA_DClip"},
		{SA_Depth,"SA_Depth"},
		{SA_DetailPen,"SA_DetailPen"},
		{SA_DisplayID,"SA_DisplayID"},
		{SA_Draggable,"SA_Draggable"},
		{SA_ErrorCode,"SA_ErrorCode"},
		{SA_Exclusive,"SA_Exclusive"},
		{SA_Font,"SA_Font"},
		{SA_FrontChild,"SA_FrontChild"},
		{SA_FullPalette,"SA_FullPalette"},
		{SA_Height,"SA_Height"},
		{SA_Interleaved,"SA_Interleaved"},
		{SA_LikeWorkbench,"SA_LikeWorkbench"},
		{SA_MaxWindowBox,"SA_MaxWindowBox"},
		{SA_MinimizeISG,"SA_MinimizeISG"},
		{SA_Obsolete1,"SA_Obsolete1"},
		{SA_OffScreenDragging,"SA_OffScreenDragging"},
		{SA_Overscan,"SA_Overscan"},
		{SA_Parent,"SA_Parent"},
		{SA_Pens,"SA_Pens"},
		{SA_PubName,"SA_PubName"},
		{SA_PubSig,"SA_PubSig"},
		{SA_PubTask,"SA_PubTask"},
		{SA_Quiet,"SA_Quiet"},
		{SA_Reserved,"SA_Reserved"},
		{SA_Reserved2,"SA_Reserved2"},
		{SA_Reserved3,"SA_Reserved3"},
		{SA_SharePens,"SA_SharePens"},
		{SA_ShowTitle,"SA_ShowTitle"},
		{SA_SysFont,"SA_SysFont"},
		{SA_Title,"SA_Title"},
		{SA_Top,"SA_Top"},
		{SA_Type,"SA_Type"},
		{SA_VideoControl,"SA_VideoControl"},
		{SA_Width,"SA_Width"},
		{SA_WindowDropShadows,"SA_WindowDropShadows"},
		{SA_Left,"SA_Left"}};

void print_tag(struct debug_tag_info *nameList, ULONG tag, LONG value )
{
	struct debug_tag_info *i;

	for ( i = nameList ; i -> name != NULL; i++)
	{
		if (i -> tag == tag)
		{
			Printf("%s: %08lx / %ld \n",i->name,value,value);
			return;
		}
	}

	return;
}

void dump_tags( const struct TagItem * tagList , struct debug_tag_info **nameList)
{
	const struct TagItem * tag;

	for (tag = tagList; (tag -> ti_Tag != TAG_DONE) || (tag -> ti_Tag != TAG_END); tag++)
	{
		print_tag(nameList, tag -> ti_Tag , tag -> ti_Data );
	}
}


