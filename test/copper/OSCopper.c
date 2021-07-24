->OSCopper.e
->Native graphics example using OS friendly copperlist

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
    viewport:PTR TO viewport,linestart,lines,backrgb,width,rport

PROC main() HANDLE
  KickVersion(37)

  screen:=OpenScreenTagList(NIL,[SA_TITLE,'OS Copper',
->                                 SA_PENS,[-1]:INT,
->                                 SA_DEPTH,4,
								 NIL])
  linestart:=screen.barheight+1
  lines:=screen.height-linestart
  width:=screen.width

  window:=OpenWindowTagList(NIL,[WA_IDCMP,IDCMP_MOUSEBUTTONS,
                                WA_FLAGS,WFLG_NOCAREREFRESH OR
                                         WFLG_ACTIVATE OR
                                         WFLG_BORDERLESS OR
                                         WFLG_BACKDROP,
                                WA_CUSTOMSCREEN,screen,NIL])

  viewport:=ViewPortAddress(window)
  rport:=window.rport
  backrgb:=Int(viewport.colormap.colortable)
  SetStdRast(rport)
  Box(0,linestart,width-1,screen.height-1)

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

  WaitLeftMouse(window)
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
