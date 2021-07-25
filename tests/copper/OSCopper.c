// OSCopper.e
// Native graphics example using OS friendly copperlist

#include <graphics/gfxbase>
#include <graphics/gfxmacros>
#include <graphics/copper>
#include <graphics/view>
#include <hardware/custom>
#include <intuition/intuition>
#include <intuition/screens>
#include <exec/memory>

bool initScreen()
{
    screen=OpenScreenTagList(NULL,
                SA_TITLE,'OS Copper',
                SA_PENS,[-1]:INT,
                SA_DEPTH,4,
                TAG_END);

    if (!screen) return false;

    window=OpenWindowTags(NULL,[WA_IDCMP,IDCMP_MOUSEBUTTONS,
                                WA_FLAGS,WFLG_NOCAREREFRESH |
                                         WFLG_ACTIVATE |
                                         WFLG_BORDERLESS |
                                         WFLG_BACKDROP,
                                WA_CUSTOMSCREEN,screen,NULL);

    if (!window) return false;

    myucoplist=AllocMem(SIZEOF ucoplist,MEMF_PUBLIC OR MEMF_CLEAR);

    if (!myucoplist) return false;  

    return true;
}

void closeDown()
{
    if (window)
    {
        if (viewport.ucopins)
        {
            FreeVPortCopLists(viewport)
            RemakeDisplay()
        }
        CloseWindow(window)
        window = NULL;
    }

    if (screen)
    {
        CloseScreen(screen);
        screen = NULL;  
    }

    if (myucoplist)
    {
        FreeVec(myucoplist);
        myucoplist = NULL;

    }
}

void errors()
{
    if (!screen) PrintF('Unable to open screen.\n');
    if (!window) PrintF('Unable to open window.\n');
    if (!myucoplist) PrintF('Unable to allocate myucoplist memory.\n');
}

int main()
{
    if (initScreen())
    {
        int linestart=screen.barheight+1;
        int lines=screen.height-linestart;
        int width=screen.width;
    
        viewport=ViewPortAddress(window);
        rport=window.rport;
        backrgb=Int(viewport.colormap.colortable);
        SetStdRast(rport);
        Box(0,linestart,width-1,screen.height-1);
        
        CINIT(myucoplist,lines*4);
    
        for (i=linestart;i<lines;i++)
        {
            CWAIT(myucoplist,i,0)
            CMOVEA(myucoplist,BPLCON3,0)
            CMOVEA(myucoplist,COLOR+2,(i-linestart) & 0xFFF)
            CMOVEA(myucoplist,BPLCON3,0x200)
            CMOVEA(myucoplist,COLOR+2,(0xFFF-i) & 0xFFF)
        }
    
        CWAIT(myucoplist,i,0);
        CMOVEA(myucoplist,COLOR+2,backrgb);
        CEND(myucoplist);
    
        Forbid();
        viewport.ucopins=myucoplist;
        Permit();
        RethinkDisplay();
        WaitLeftMouse(window);
    }
    else
    {
        errors();       
    }

    closeDown();
  
    return 0;
}
