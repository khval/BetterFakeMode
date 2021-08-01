

#define GetFakeScreen(n) (screens + n)

extern struct Screen screens[max_screens];
extern struct Layer_Info *LayerInfos[max_screens];
extern bool allocatedScreen[max_screens];

extern int alloc_screen_in_list();
extern void remove_screen_from_list(struct Screen *screen);
extern bool is_fake_screen( struct Screen *screen );

//extern struct Screen *first_fake_screen();

extern struct modeT * is_leagcy_mode( ULONG id );
extern struct modeT * legacy_in_tags( const struct TagItem * tagList, bool legacy_status_maybe );
extern struct modeT *bestMode( ULONG monitor, ULONG w, ULONG h);

struct Screen *current_fake_screen();
void fake_screen_to_front(struct Screen *s);
void fake_screen_to_back(struct Screen *s);

