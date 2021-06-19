

struct Gadget	*glist_head[1000];
struct Gadget	*glist_next[1000];

struct Gadget *find_blitz_gadget(int list,int nr)
{
	struct Gadget *gad;

	gad = glist_head[list];

	while ((nr!=0)&&(gad!=0))
	{
		if (gad -> GadgetID == nr) return gad;	
		gad = gad -> NextGadget;	
	}
	return NULL;
}


void GTDisable(int list,int nr, void *win)
{
	struct Gadget *gad;

	if (gad=find_blitz_gadget(list,nr))
	{
		GT_SetGadgetAttrs(gad,win,NULL,
			GA_Disabled, TRUE,
			TAG_DONE);
	}
}

void GTEnable(int list,int nr, void *win)
{
	struct Gadget *gad;

	if (gad=find_blitz_gadget(list,nr))
	{
		GT_SetGadgetAttrs(gad,win,NULL,
			GA_Disabled, FALSE,
			TAG_DONE);
	}
}

void init_blitz_gedtools()
{
	int i;

	for(i=0;i<(sizeof(glist_head) / sizeof(struct Gadget *) );i++)
	{
		glist_head[i]=0;
		glist_next[i]=0;
	}
}


void fix_gtlist(int glist)
{
	if (glist_head[glist]==0)	{glist_next[glist] = CreateContext( (void *) &glist_head[glist] ); }
}


int AttachGTList(int glist,int winnr)
{
	if (glist_head[glist])
	{

//struct Window *, struct Gadget *

#if __USE_EXT_BLITZ_WINDOW__
		AddGList( (struct Window *) win[winnr].win,(struct Gadget *) glist_head[glist],0,-1,NULL);
		RefreshGList( (void *) glist_head[glist],(void *) win[winnr].win,0,-1); 
#else
		AddGList( (struct Window *) win[winnr],(struct Gadget *) glist_head[glist],0,-1,NULL);
		RefreshGList( (void *) glist_head[glist],(void *) win[winnr],0,-1); 
#endif

	}
	else
	{
		printf("\nsome thing wrong some where\nhappy hounting\n");
	}
}

void FreeGTList(int glist)
{
	if (glist_head[glist])
	{
		FreeGadgets(glist_head[glist]);
		glist_head[glist] = 0;
		glist_next[glist] = 0;
	}
}

void gt_add_item(struct List *list,char *text)
{
	struct Node *NewNode;

	if (NewNode = (void *) malloc(sizeof(struct Node)))
	{
		bzero(NewNode,sizeof(struct Node));
		AddTail(list,NewNode);
		NewNode -> ln_Name = text;
	}	
}

void GT_configure( struct NewGadget *ng,int id,int x,int y,int w,int h)
{
	bzero(ng,sizeof(struct NewGadget));


// 	printf("%x\n", GetVisualInfo(src, TAG_DONE ));


	ng -> ng_LeftEdge = x;
	ng -> ng_TopEdge = y;
	ng -> ng_Width = w;
	ng -> ng_Height = h;
	ng -> ng_GadgetID = id;
	ng -> ng_VisualInfo = vi;
//	ng -> ng_VisualInfo = NULL;
	ng -> ng_TextAttr = 0;

	if (ng -> ng_VisualInfo == NULL)
	{
		printf("Screen has none working VisualInfo\n");
	}

/*
	if (I_Intuition -> GetScreenAttr(src,SA_Font,ng -> ng_TextAttr,sizeof(struct TextAttr) )==0)
	{
		printf("Can't get TextAttr\nForced!!\n");
		ng -> ng_TextAttr = src -> Font;
		
		printf("Screen font %x\n",src -> Font);
	}
*/
	ng -> ng_Flags = 0;
}



void GTText(int list,int id, int x,int y,int w,int h,char *text)
{
	struct NewGadget ng;

	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;
	ng.ng_Flags = PLACETEXT_IN | NG_HIGHLABEL;

	fix_gtlist(list);
	glist_next[list] =  CreateGadget(TEXT_KIND, glist_next[list], &ng, TAG_DONE);
}

void GTButton(int list,int id,int x,int y,int w,int h,char *text, int flags)
{
	struct NewGadget ng;
	void			 *ok;

	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;

	fix_gtlist(list);
	ok = CreateGadget(BUTTON_KIND, glist_next[list], &ng,TAG_DONE);

	if (ok)
	{ glist_next[list] = ok; }
	 else
	{ printf("GTButton failed x = %i y = %i w = %i h = %i \n",x,y,w,h); }
}

void ShapeGadget(int list,int x,int y,int flags,int id,int i1,int i2)
{
	GTButton(list,id,x,y,70,30,"Start'96",flags);
}

void GTCycle(int list,int id, int x,int y,int w,int h,char *text,int flag,char **labels, int value)
{
	struct NewGadget ng;
	void *ok;

	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;
	ng.ng_Flags = NG_HIGHLABEL;

	fix_gtlist(list);

	ok = CreateGadget(CYCLE_KIND, glist_next[list], &ng,
		GTCY_Labels, labels,
		GTCY_Active, value, 
		TAG_DONE);

	if (ok) { glist_next[list] = ok; } else { printf("GTCycle failed x = %i y = %i w = %i h = %i \n",x,y,w,h); }
}

void GTCheckBox(int list,int id,int x,int y,int w,int h,char *text)
{
	struct NewGadget ng;

	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;

	fix_gtlist(list);
	glist_next[list] = CreateGadget(CHECKBOX_KIND, glist_next[list], &ng,
		GTCB_Scaled, TRUE,
		TAG_DONE);
}

void GTInteger(int list,int id,int x,int y,int w,int h,char *text, int flags,int value)
{
	struct NewGadget ng;

	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;

	fix_gtlist(list);
	glist_next[list] = CreateGadget(INTEGER_KIND, glist_next[list], &ng,
		GTIN_Number, value,
		GTIN_MaxChars, 10,
		TAG_DONE);
}

struct Gadget *GT_SliderGadget(int id,struct Gadget *last, int x,int y,int w,int h,char *text,int min,int max,int level)
{
	struct NewGadget ng;

	GT_configure ( &ng, id,x,y,w,h);

//	fix_gtlist(list);
	return CreateGadget(SLIDER_KIND, last, &ng,
		GTSL_Min, min,
		GTSL_Max, max,
		GTSL_Level, level,
		GTSL_LevelFormat, "%2ld",
		GTSL_LevelPlace, PLACETEXT_LEFT,
		GTSL_MaxLevelLen, 2,
		GA_Immediate, TRUE,
		GA_RelVerify, TRUE,
		TAG_DONE);
}

/* 

GTScroller(win_nr,200,19,-10+WB96OFFSET,16,WindowHeight-2,"",417 | 400 ,windir(win_nr,5)-1,windir(win_nr,4)-1,0); 
                    list     id    x                   y              w         h                  txt, flags,                       a                    b                   c
*/

void GTScroller(int list,int id,int x,int y,int w,int h,char *text,int flags,int a,int b,int c)
{
	struct NewGadget ng;

	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;

	printf("Before glist_head = 0x%X\n",glist_head[list]);
	printf("Before glist_next = 0x%X\n",glist_next[list]);

	fix_gtlist(list);

	printf("After glist_head = 0x%X\n",glist_head[list]);
	printf("After glist_next = 0x%X\n",glist_next[list]);


	glist_next[list] = CreateGadget(SCROLLER_KIND, glist_next[list], &ng,
		GTSC_Top, c,
		GTSC_Total, b,
		GTSC_Visible, a,
		GTSC_Arrows, 13,
		GACT_IMMEDIATE, TRUE,
		GACT_RELVERIFY, TRUE, 
		PGA_Freedom, flags,
		TAG_DONE);

}

struct Gadget *GT_NumberGadget(int id,struct Gadget *last, int x,int y,int w,int h,char *text)
{
	struct NewGadget ng;

	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;

//	fix_gtlist(list);
	return CreateGadget(NUMBER_KIND, last, &ng,
		GTNM_Number, 314159,
		TAG_DONE);
}

struct Gadget *GT_MxGadget(int id,struct Gadget *last, int x,int y,int w,int h,char *text)
{
	struct NewGadget ng;
	
	GT_configure ( &ng, id,x,y,w,h);

//	fix_gtlist(list);
	return CreateGadget(MX_KIND, last, &ng,
//		GTMX_Labels, DayLabels,
		GTMX_Active, 0,
		GTMX_Spacing, 4,
		TAG_DONE);
}

struct Gadget *GTString(int list,int id,int x,int y,int w,int h,char *label,int flags, int maxchr,char *text)
{
	struct NewGadget ng;
	
	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = label;
	
	fix_gtlist(list);
	glist_next[list] = CreateGadget(STRING_KIND, glist_next[list], &ng,
		GTST_MaxChars, maxchr,
		GTST_String, text,
		TAG_DONE);
}

struct Gadget *GT_StringGadget(int id,struct Gadget *last, int x,int y,int w,int h,char *text)
{
	struct NewGadget ng;
	
	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;
	
//	fix_gtlist(list);
	return CreateGadget(STRING_KIND, last, &ng,
		GTST_MaxChars, 50,
		TAG_DONE);
}

struct Gadget *GT_ListGadget(int id,struct Gadget *last, int x,int y,int w,int h,char *text,struct List *list)
{
	struct NewGadget ng;
	
	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;
	ng.ng_Flags = NG_HIGHLABEL|PLACETEXT_LEFT;
    
//	fix_gtlist(list);
	return CreateGadget(LISTVIEW_KIND, last, &ng,
		GTLV_Labels, &list,  
		GTLV_Top, 1,
		LAYOUTA_Spacing, 1,
		GTLV_Selected, 3,
		GTLV_ScrollWidth, 18,
		TAG_DONE);
}


struct Gadget *GT_PalletteGadget(int id,struct Gadget *last, int x,int y,int w,int h,char *text)
{
	struct NewGadget ng;

	GT_configure ( &ng, id,x,y,w,h);
	ng.ng_GadgetText = text;
	ng.ng_Flags = NG_HIGHLABEL;

//	fix_gtlist(list);
	return CreateGadget(PALETTE_KIND, last , &ng,
		GTPA_Depth, 0,
		GTPA_Color, 1,
		GTPA_ColorOffset, 0,
		GTPA_IndicatorHeight, 15,
		TAG_DONE);
}
