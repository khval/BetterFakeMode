
extern struct Screen screens[max_screens];
extern bool allocatedScreen[max_screens];

extern int alloc_screen_in_list();
extern void remove_screen_from_list(struct Screen *screen);
extern bool is_fake_screen( struct Screen *screen );
extern struct Screen *first_fake_screen();
extern bool is_leagcy_mode( ULONG id );
extern bool legacy_in_tags( const struct TagItem * tagList, bool legacy_status_maybe );


