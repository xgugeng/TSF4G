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


typedef struct _tdtp_instance_t
{
	struct sockaddr_in  listenaddr;
	int					listenfd;
	
	int					epollfd;

//有数据需要读取的socket
    TLIBC_LIST_HEAD     readable_list;
    
	int					input_tbusid;
	tbus_t				*input_tbus;
	
	int					output_tbusid;
	tbus_t				*output_tbus;

	tlibc_timer_t		timer;
	tuint64				timer_start_ms;

	tlibc_mempool_t		*socket_pool;
	tuint32				socket_pool_size;

	tlibc_mempool_t		*package_pool;
	tuint32				package_pool_size;
}tdtp_instance_t;



TERROR_CODE tdtp_instance_init(tdtp_instance_t *self);

TERROR_CODE tdtp_instance_process(tdtp_instance_t *self);


#endif //_H_INSTANCE_H

