#include "tconnd/tdtp/timer/tdtp_timer_accept_timeout.h"
#include "tlibc/platform/tlibc_platform.h"
#include "tlibc/core/tlibc_mempool.h"
#include "tconnd/tdtp/timer/tdtp_timer.h"
#include <unistd.h>



void tdtp_timer_accept_timeout(const tlibc_timer_entry_t *super)

{
    tdtp_timer_accept_timeout_t *self = &TLIBC_CONTAINER_OF(super, tdtp_timer_t, entry)->acccept_timeout;
    
    close(self->socketfd);
    tlibc_mempool_free(self->mp, self->mid);
}

