->OSCopper.e
->Native graphics example using OS friendly copperlist

OPT PREPROCESS

MODULE 'graphics/gfxbase',
       'graphics/gfxmacros',
       'graphics/copper',
       'graphics/view',
       'graphics/gfx',
       'graphics/rastport',
       'hardware/custom',
       'intuition/intuition',
       'intuition/screens',
       'exec/memory'

ENUM ERR_NONE,ERR_SCREEN,ERR_WINDOW

RAISE ERR_SCREEN IF OpenScreenTagList()=NIL,
      ERR_WINDOW IF OpenWindowTagList()=NIL,
      "MEM"      IF AllocMem()=NIL

DEF screen=NIL:PTR TO screen,window=NIL:PTR TO window,myucoplist=NIL,i,
    viewport:PTR TO viewport,linestart,lines,backrgb,width,rport:PTR TO rastport,
    bitmap:PTR TO bitmap,modulo,planesize,bitplane

PROC main() HANDLE
  DEF x,y

  KickVersion(37)

  screen:=OpenScreenTagList(NIL,[SA_TITLE,'OS Copper',
->                               SA_PENS,[-1]:INT,
->                               SA_DEPTH,4,
                                 SA_WIDTH, 320,
                                 SA_HEIGHT, 256,

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
  backrgb:=Int(viewport.colormap.colortable)
  rport:=window.rport
  bitmap:=screen.rastport.bitmap
  modulo:=bitmap.bytesperrow-40
  planesize:=modulo*screen.height
  bitplane:=bitmap.planes[0]

  SetStdRast(rport)
  SetColour(screen,0,0,0,0)
  SetColour(screen,1,255,255,255)
->  SetRast(rport,1)
->  Box(0,linestart,width-1,screen.height-1)

 FOR y:=0 TO 64 STEP 64
    FOR x:=0 TO 256 STEP 64
    RectFill(rport,x,y,x+31,y+31)
->      Box(x,y,32,32,1)
    ENDFOR
   FOR x:=32 TO 288 STEP 64
    RectFill(rport,x,y+32,x+31,y+63)
->      Box(x,y+32,32,32,1)
    ENDFOR
  ENDFOR

  myucoplist:=AllocMem(SIZEOF ucoplist,MEMF_PUBLIC OR MEMF_CLEAR)
  CINIT(myucoplist,lines*4)
  CMOVEA(myucoplist,COLOR+2,$FFF)
  FOR i:=linestart TO lines
  CWAIT(myucoplist,i,0)
  IF i=127 THEN CMOVEA(myucoplist,BPL1MOD,-1*(planesize/2))
  IF i=128 THEN CMOVEA(myucoplist,BPL1MOD,modulo)
->  IF i=127
->    CMOVEA(myucoplist,BPLPT,Shr(bitplane,16))
->    CMOVEA(myucoplist,BPLPT+2,bitplane AND $FFFF)
->  ENDIF
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
