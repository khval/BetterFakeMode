
struct ewindow {
	struct Window *win;
	struct Layer_Info *layer_info;
	struct ClipRect *alpha_clip;
	struct BitMap *background;
	struct TextFont *org_font;
};

#define max_windows 2

extern struct Window	*win[max_windows];
extern struct Screen		*src;

extern struct Gadget		*GadgetHit;
extern struct Window	*current_win;
extern unsigned int		EventCode;
extern int				EventWindow;


