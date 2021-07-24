
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <exec/emulation.h>
#include <exec/ports.h>

#include "Engine.h"
#include "EngineTimer.h"

int frame_skip = 0;
int line_skip = 0;

void reset_timer(struct TimeRequest *timer_io)
{
	timer_io->Request.io_Command = TR_ADDREQUEST;
	timer_io->Time.Seconds = 0;
	timer_io->Time.Microseconds = 16667 * (frame_skip+1);
	SendIO((struct IORequest *)timer_io);
}

bool open_timer_context( struct TimerContext *tc )
{
	tc -> timer_port = (struct MsgPort*) AllocSysObjectTags(ASOT_PORT, TAG_DONE);

	if (tc -> timer_port)
	{
		tc -> timer_io = (struct TimeRequest *) 
				AllocSysObjectTags(ASOT_IOREQUEST, 
					ASOIOR_Size, sizeof(struct TimeRequest),
					ASOIOR_ReplyPort, tc-> timer_port ,
					TAG_DONE);

		if (tc -> timer_io)
		{
			if (!OpenDevice( (char *) TIMERNAME, UNIT_MICROHZ, (struct IORequest *) tc -> timer_io, 0))
			{
				tc -> timer_mask = 1 << tc -> timer_port->mp_SigBit;
				reset_timer( tc -> timer_io );
				tc -> its_open = true;
				return true;
			}
		}
	}
	return false;
}

void close_timer_context( struct TimerContext *tc )
{
	if (tc->its_open)
	{
		Wait( tc->timer_mask );
		CloseDevice( (struct IORequest *) tc -> timer_io );
		tc->its_open = false;
	}

	if (tc -> timer_io)
	{
     		FreeSysObject(ASOT_IOREQUEST, (APTR) tc -> timer_io);
		tc -> timer_io = NULL;
	}

	if (tc -> timer_port)
	{
     		FreeSysObject(ASOT_PORT, (APTR) tc -> timer_port);
		tc -> timer_port = NULL;
	}
}


