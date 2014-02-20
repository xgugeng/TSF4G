#include "tbus/tbus.h"
#include "tcommon/tdgi_types.h"
#include "tcommon/tdgi_writer.h"
#include "tcommon/tdgi_reader.h"
#include "tcommon/tdtp.h"

#include "tlibc/protocol/tlibc_binary_reader.h"
#include "tlibc/protocol/tlibc_binary_writer.h"

#define TLOG_INSTANCE_LEVEL e_tlog_info
#include "tlog/tlog_instance.h"


#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>


#define iSHM_KEY 10002
#define oSHM_KEY 10001


const char *message = NULL;

tbus_t *itb;
tbus_t *otb;

#define TBUS_MTU 65536
void block_send_pkg(tbus_t *tb, const tdgi_rsp_t *pkg, const char* data, size_t data_size)
{
    char *addr;
    size_t len = 0;
    TERROR_CODE ret;
    TLIBC_ERROR_CODE r;
    int idle = 0;

    TLIBC_BINARY_WRITER writer;
    char buff[TBUS_MTU];
    size_t buff_len;

    DEBUG_PRINT("block_send_pkg pkg.cmd = %d, pkg.mid_num = %u, pkg.mid[0]=%llu pkg.size = %u data_size = %zu"
        , pkg->cmd, pkg->mid_num, pkg->mid[0], pkg->size, data_size);
        
    tlibc_binary_writer_init(&writer, buff, TBUS_MTU);    
    r = tlibc_write_tdgi_rsp_t(&writer.super, pkg);
    if(r != E_TLIBC_NOERROR)
    {
        ERROR_PRINT("tlibc_write_tdgi_rsp_t return [%d]", r);
        exit(1);
    }
    buff_len = writer.offset;

    len = buff_len + data_size;
    for(;;)
    {
        ret = tbus_send_begin(tb, &addr, &len);
        if(ret == E_TS_NOERROR)
        {
            assert(len >= buff_len);
            memcpy(addr, buff, buff_len);
            addr += buff_len;
            if(data)
            {
                memcpy(addr, data, data_size);
            }
            
            tbus_send_end(tb, buff_len + data_size);
            break;
        }
        else if(ret == E_TS_WOULD_BLOCK)
        {
            ++idle;
            if(idle > 30)
            {
                usleep(1000);
            }
        }
        else
        {
            ERROR_PRINT("tbus_send_begin return [%d]", ret);
            exit(1);
        }        
    }
}


tdgi_size_t process_pkg(const tdgi_req_t *req,  const char* body_ptr)
{
    tdgi_rsp_t rsp;
    TLIBC_UNUSED(body_ptr);
    
    switch(req->cmd)
    {
    case e_tdgi_cmd_connect:
        rsp.cmd = e_tdgi_cmd_accept;
        rsp.mid_num = 1;
        rsp.mid[0] = req->mid;
        rsp.size = 0;
        block_send_pkg(otb, &rsp, NULL, 0);
        INFO_PRINT("[%llu] connect.", req->mid);
        break;
    case e_tdgi_cmd_recv:
        if(req->size == 0)
        {
            INFO_PRINT("[%llu] client close.", req->mid);
            return 0;
        }
        else
        {
            if(rand() % 100 < 0)
            {
                rsp.cmd = e_tdgi_cmd_close;
                rsp.mid_num = 1;
                rsp.mid[0] = req->mid;
                rsp.size = 0;
                block_send_pkg(otb, &rsp, NULL, 0);
                INFO_PRINT("[%llu] server close.", req->mid);
            }
            else
            {
                const char *iter, *limit, *next;
                rsp.cmd = e_tdgi_cmd_send;
                rsp.mid_num = 1;
                rsp.mid[0] = req->mid;
                
                limit = body_ptr + req->size;
                for(iter = body_ptr; iter < limit; iter = next)
                {
                    tdtp_size_t pkg_size = *(tdtp_size_t*)iter;
                    const char* pkg_content = iter + sizeof(tdtp_size_t);
                    next = iter + sizeof(tdtp_size_t) + pkg_size;                    
                    DEBUG_PRINT("[%llu] recv pkg_size: %u, pkg_content: %s.", req->mid, pkg_size, pkg_content);

                    rsp.size = pkg_size;

                    block_send_pkg(otb, &rsp, pkg_content, pkg_size);
                }
            }
            return req->size;
        }
    default:
        break;
    }
    return 0;
}

int g_sig_term = FALSE;
static void on_signal(int sig)
{
    switch(sig)
    {
        case SIGINT:
        case SIGTERM:
            g_sig_term = true;
            break;
    }
}


int main()
{
    tdgi_req_t pkg;
	int ishm_id, oshm_id;
	size_t len;
	size_t message_len;
	TERROR_CODE ret;
	TLIBC_BINARY_READER reader;
	TLIBC_ERROR_CODE r;
    struct sigaction  sa;

	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_handler = SIG_IGN;
    if(sigaction(SIGPIPE, &sa, NULL) != 0)
    {
        ERROR_PRINT("sigaction error[%d], %s.", errno, strerror(errno));
        exit(1);
    }

    sa.sa_handler = on_signal;
    if((sigaction(SIGTERM, &sa, NULL) != 0)
        ||(sigaction(SIGINT, &sa, NULL) != 0))
    {
        ERROR_PRINT("sigaction error[%d], %s.", errno, strerror(errno));
        exit(1);
    }


    
	ishm_id = shmget(iSHM_KEY, 0, 0666);
    itb = shmat(ishm_id , NULL, 0);

	
	oshm_id = shmget(oSHM_KEY, 0, 0666);
    otb = shmat(oshm_id, NULL, 0);

    srand(time(0));


	for(;!g_sig_term;)
	{
		ret = tbus_read_begin(itb, &message, &message_len);
		if(ret == E_TS_NOERROR)
		{
		    len = message_len;
		    while(len > 0)
		    {
		        tdgi_size_t body_size;
		        
    			tlibc_binary_reader_init(&reader, message, len);
	    		r = tlibc_read_tdgi_req_t(&reader.super, &pkg);
	    		if(r != E_TLIBC_NOERROR)
	    		{
    	    		ERROR_PRINT("tlibc_read_tdgi_req_t error");
	    		    exit(1);
	    		}		
                body_size = process_pkg(&pkg, message + reader.offset);
                len -= reader.offset + body_size;
                message += reader.offset + body_size;
	    	}			
			tbus_read_end(itb, message_len);
			continue;
		}
		else if(ret == E_TS_WOULD_BLOCK)
		{
			usleep(1000);
		}
		else
		{
			ERROR_PRINT("tbus_read_begin error.");
			exit(1);
		}
	}
	
	return 0;
}

