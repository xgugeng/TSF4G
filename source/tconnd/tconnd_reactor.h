#ifndef _H_TCONND_REACTOR_H
#define _H_TCONND_REACTOR_H

#include "tconnd/tconnd_config_types.h"
#include "tcommon/terrno.h"
#include "tbus/tbus.h"

#include "tlibc/core/tlibc_list.h"
#include "tlibc/core/tlibc_mempool.h"


#include <sys/epoll.h>
#include <netinet/in.h>

extern int g_tconnd_reactor_switch;


TERROR_CODE tconnd_reactor_init(const char* config_file);

TERROR_CODE tconnd_reactor_process();

void tconnd_reactor_loop();

void tconnd_reactor_fini();

#endif //_H_TCONND_REACTOR_H

