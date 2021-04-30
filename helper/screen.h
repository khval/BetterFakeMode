
extern struct Screen screens[max_screens];
extern bool allocatedScreen[max_screens];

extern int alloc_screen_in_list();
extern void remove_screen_from_list(struct Screen *screen);

