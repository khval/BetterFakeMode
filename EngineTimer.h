

struct TimerContext
{
	struct MsgPort *timer_port ;
	struct TimeRequest *timer_io ;
	bool its_open;
	ULONG timer_mask ;
};

void reset_timer(struct TimeRequest *timer_io);
bool open_timer_context( struct TimerContext *tc );
void close_timer_context( struct TimerContext *tc );

