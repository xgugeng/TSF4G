#include "tconnd_timer.h"
#include "tconnd/tdtp_instance.h"

#include <sys/time.h>


tuint64 get_current_ms()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);

	return tv.tv_sec*1000 + tv.tv_usec/1000;
}

tuint64 tdtp_instance_get_time_ms(tdtp_instance_t *self)
{
	return get_current_ms() - self->timer_start_ms;
}

