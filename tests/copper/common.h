
typedef void * PTR;

extern struct Screen *screen;
extern struct Window *window;
extern struct ViewPort *viewport;
extern PTR myucoplist;

extern void Box(struct RastPort *rp, int x0,int y0, int x1,int y1, int color);

#define CMOVEA(c,a,b) { CMove(c,a,b);CBump(c); }
#define COLOR(n) (0x180 + (n <<1))
#define BPLCON3 0x106

#define SetColour(src,a,r,g,b) SetRGB32( &(src -> ViewPort), (ULONG) a*0x01010101, (ULONG) r*0x01010101,(ULONG) g*0x01010101,(ULONG) b*0x01010101 )

extern void dumpUCopList( struct UCopList *copper );
extern void dumpCopList( struct CopList *cl );
extern void dumpCopIns( struct CopIns *c, int cnt );
extern bool checkMouse(ULONG bcode);

