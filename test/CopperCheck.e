->CopperCheck
->Native graphics example using OS friendly copperlist
->Displays checker pattern then scrolls each line indivdiually

OPT PREPROCESS

MODULE 'graphics/gfxbase',
       'graphics/gfxmacros',
       'graphics/copper',
       'graphics/view',
       'hardware/custom',
       'intuition/intuition',
       'intuition/screens',
       'exec/memory'

ENUM ERR_NONE,ERR_SCREEN,ERR_WINDOW

RAISE ERR_SCREEN IF OpenScreenTagList()=NIL,
      ERR_WINDOW IF OpenWindowTagList()=NIL,
      "MEM"      IF AllocMem()=NIL

DEF screen=NIL:PTR TO screen,window=NIL:PTR TO window,myucoplist=NIL,i,
    viewport:PTR TO viewport,linestart,lines,backrgb,rport,x,y,
    class

PROC main() HANDLE
  KickVersion(37)

  screen:=OpenScreenTagList(NIL,[SA_QUIET,TRUE,
                                 SA_DEPTH,1,
                                 SA_WIDTH,320,
                                 SA_HEIGHT,256,
->                                 SA_COLORS,[0,$FFF,0,0]:INT,
                                 SA_TYPE,CUSTOMSCREEN,
                                 NIL])

  window:=OpenWindowTagList(NIL,[WA_IDCMP,IDCMP_MOUSEBUTTONS,
                                 WA_FLAGS,WFLG_NOCAREREFRESH OR
                                          WFLG_ACTIVATE OR
                                          WFLG_BORDERLESS OR
                                          WFLG_BACKDROP OR
                                          WFLG_RMBTRAP,
                                 WA_CUSTOMSCREEN,screen,
                                 WA_LEFT,0,
                                 WA_TOP,0,
                                 WA_WIDTH,320,
                                 WA_HEIGHT,256,NIL])


  viewport:=ViewPortAddress(window)
  rport:=window.rport
  backrgb:=Int(viewport.colormap.colortable)
  SetStdRast(rport)
  SetColour(screen,0,0,0,0)
  SetColour(screen,1,255,255,255)

  FOR y:=0 TO 256 STEP 64
    FOR x:=0 TO 192 STEP 64
      Box(x,y,32,32,1)
    ENDFOR
  ENDFOR

  myucoplist:=AllocMem(SIZEOF ucoplist,MEMF_PUBLIC OR MEMF_CLEAR)
  CINIT(myucoplist,lines*4)
  FOR i:=linestart TO lines
  CWAIT(myucoplist,i,0)
  CMOVEA(myucoplist,BPLCON3,0)
  CMOVEA(myucoplist,COLOR+2,(i-linestart) AND $FFF)
  CMOVEA(myucoplist,BPLCON3,$200)
  CMOVEA(myucoplist,COLOR+2,($FFF-i) AND $FFF)
  ENDFOR
  CWAIT(myucoplist,i,0)
  CMOVEA(myucoplist,COLOR+2,backrgb)
  CEND(myucoplist)

  Forbid()
  viewport.ucopins:=myucoplist
  Permit()
  RethinkDisplay()

  REPEAT
    class:=WaitIMessage(window)
  UNTIL (class=IDCMP_MOUSEBUTTONS) AND (MsgCode()=MENUUP)
EXCEPT DO
  IF window
    IF viewport.ucopins
      FreeVPortCopLists(viewport)
      RemakeDisplay()
    ENDIF
    CloseWindow(window)
  ENDIF

  IF screen THEN CloseScreen(screen)

  SELECT exception
  CASE ERR_SCREEN; PrintF('Unable to open screen.\n')
  CASE ERR_WINDOW; PrintF('Unable to open window.\n')
  CASE "MEM";      PrintF('Unable to allocate some memory.\n')
  ENDSELECT
ENDPROC
