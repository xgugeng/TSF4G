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


#define TDTP_MAX_EVENTS 1024

typedef struct _tdtp_instance_t
{
	const tconnd_tdtp_t *config;

	struct sockaddr_in  listenaddr;
	int					listenfd;
	
	int					epollfd;
	
	struct epoll_event 	events[TDTP_MAX_EVENTS];
	int					events_num;
    
	int					input_tbusid;
	tbus_t				*input_tbus;
	
	int					output_tbusid;
	tbus_t				*output_tbus;

	tlibc_timer_t		timer;
	tuint64				timer_start_ms;

	tlibc_mempool_t		*socket_pool;
	tuint32				socket_pool_size;


	tlibc_mempool_t		*timer_pool;
	tuint32				timer_pool_size;
}tdtp_instance_t;



TERROR_CODE tdtp_instance_init(tdtp_instance_t *self, const tconnd_tdtp_t *config);

TERROR_CODE tdtp_instance_process(tdtp_instance_t *self);


#endif //_H_INSTANCE_H

