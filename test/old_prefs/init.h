
extern struct Library			*IntuitionBase;
extern struct IntuitionIFace	*IIntuition;

extern struct Library			*GraphicsBase;
extern struct GraphicsIFace	*IGraphics;

extern struct Library			*ASLBase;
extern struct ASLIFace		*IASL;

extern struct Library			*GadToolsBase;
extern struct GadToolsIFace	*IGadTools;


extern bool open_libs();
extern void close_libs();

extern BOOL open_lib( const char *name, int ver , const char *iname, int iver, struct Library **base, struct Interface **interface);
extern void close_lib_all( struct Library **Base, struct Interface **I );

#define close_lib(name) close_lib_all( &(name ## Base), (struct Interface **) &(I ## name) )

