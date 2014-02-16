#ifndef _H_INSTANCE_H
#define _H_INSTANCE_H

#include "tconnd/tconnd_config_types.h"
#include "tcommon/terrno.h"
#include "tbus/tbus.h"

#include "tlibc/core/tlibc_list.h"
#include "tlibc/core/tlibc_mempool.h"
#include "tlibc/core/tlibc_timer.h"


#include <sys/epoll.h>
#include <netinet/in.h>

extern int g_tdtp_instance_switch;


TERROR_CODE tdtp_instance_init(const char* config_file);

TERROR_CODE tdtp_instance_process();

void tdtp_instance_loop();

void tdtp_instance_fini();

#endif //_H_INSTANCE_H

