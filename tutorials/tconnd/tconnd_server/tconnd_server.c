#include "tbus/tbus.h"
#include "tcommon/sip.h"
#include "tcommon/bscp.h"

#ifdef TLOG_PRINT_LEVEL
#undef TLOG_PRINT_LEVEL
#endif//TLOG_PRINT_LEVEL
#define TLOG_PRINT_LEVEL e_tlog_debug



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


char *message = NULL;

tbus_t *itb;
tbus_t *otb;

#define TBUS_MTU 65536
static void block_send_pkg(tbus_t *tb, const sip_rsp_t *pkg, const char* data, size_t data_size)
{
    char *addr;
    size_t len = 0;
    TERROR_CODE ret;
    int idle = 0;
    size_t head_size;


    DEBUG_PRINT("block_send_pkg pkg.cmd = %d, pkg.cid_list_num = %u, pkg.cid_list[0].id=%u, pkg.cid_list[0].sn=%"PRIu64" pkg.size = %u data_size = %zu."
        , pkg->cmd, pkg->cid_list_num, pkg->cid_list[0].id, pkg->cid_list[0].sn, pkg->size, data_size);

    head_size = SIP_RSP_T_SIZE(pkg);

    
    len = head_size + data_size;
    for(;;)
    {
        ret = tbus_send_begin(tb, &addr, (uint32_t*)&len);
        if(ret == E_TS_NOERROR)
        {
            memcpy(addr, pkg, head_size);         
            sip_rsp_t_code((sip_rsp_t*)addr);
            addr += head_size;
            if(data)
            {
                memcpy(addr, data, data_size);
            }
            
            tbus_send_end(tb, (uint32_t)(head_size + data_size));
            break;
        }
        else if(ret == E_TS_TBUS_NOT_ENOUGH_SPACE)
        {
            ++idle;
            if(idle > 30)
            {
				idle = 0;
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

#define BLOCK_SIZE 1024

char buff[BLOCK_SIZE];
static sip_size_t process_pkg(const sip_req_t *req,  const char* body_ptr)
{
    sip_rsp_t rsp;
    TLIBC_UNUSED(body_ptr);

    INFO_PRINT("req.cmd = %d req.cid = [%u, %"PRIu64"] req.size = %u.", req->cmd, req->cid.id, req->cid.sn, req->size);
    switch(req->cmd)
    {
    case e_sip_req_cmd_connect:
        rsp.cmd = e_sip_rsp_cmd_accept;
        rsp.cid_list_num = 1;
        rsp.cid_list[0] = req->cid;
        rsp.size = 0;
        block_send_pkg(otb, &rsp, NULL, 0);
        INFO_PRINT("[%u, %"PRIu64"] accept.", req->cid.id, req->cid.sn);
        break;
    case e_sip_req_cmd_recv:
        if(req->size == 0)
        {
            INFO_PRINT("[%u, %"PRIu64"] client close.", req->cid.id, req->cid.sn);
            return 0;
        }
        else
        {
            if(rand() % 100 < 0)
            {
                rsp.cmd = e_sip_rsp_cmd_close;
                rsp.cid_list_num = 1;
                rsp.cid_list[0] = req->cid;
                rsp.size = 0;
                block_send_pkg(otb, &rsp, NULL, 0);
                INFO_PRINT("[%"PRIu64"] server close.", req->cid.sn);
            }
            else
            {
                const char *iter, *limit, *next;
                rsp.cmd = e_sip_rsp_cmd_send;
                rsp.cid_list_num = 1;
                rsp.cid_list[0] = req->cid;
                
                limit = body_ptr + req->size;
                for(iter = body_ptr; iter < limit; iter = next)
                {
                    bscp_head_t pkg_size = *(const bscp_head_t*)iter;
                    const char* pkg_content = iter + sizeof(bscp_head_t);

                    bscp_head_t_decode(pkg_size);
                    
                    next = iter + BSCP_HEAD_T_SIZE + pkg_size;                    
                    DEBUG_PRINT("[%"PRIu64"] recv pkg_size: %u, pkg_content: %s.", req->cid.sn, pkg_size, pkg_content);
                    

                    rsp.size = pkg_size;

                    block_send_pkg(otb, &rsp, buff, BLOCK_SIZE);
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
    sip_req_t *pkg;
	int ishm_id, oshm_id;
	size_t len;
	size_t message_len = 0;
	TERROR_CODE ret;
    struct sigaction  sa;
	size_t idle_times = 0;
	
    INFO_PRINT("Hello world!");

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

    srand((unsigned int)time(0));


	for(;!g_sig_term;)
	{
		ret = tbus_read_begin(itb, &message, (uint32_t*)&message_len);
		if(ret == E_TS_NOERROR)
		{
		    len = message_len;
		    while(len > 0)
		    {
		        sip_size_t body_size;
		        if(len < SIP_REQ_SIZE)
		        {
                    ERROR_PRINT("tlibc_read_tdgi_req_t error");
		            exit(1);
		        }
		        pkg = (sip_req_t*)message;
		        sip_req_t_decode(pkg);

		        
                body_size = process_pkg(pkg, message + SIP_REQ_SIZE);
                len -= SIP_REQ_SIZE + body_size;
                message += SIP_REQ_SIZE + body_size;
	    	}			
			tbus_read_end(itb, (uint32_t)message_len);
			idle_times = 0;
			continue;
		}
		else if(ret == E_TS_WOULD_BLOCK)
		{
			++idle_times;
			if(idle_times > 30)
			{
				idle_times = 0;
				usleep(1000);
			}			
		}
		else
		{
			ERROR_PRINT("tbus_read_begin error.");
			exit(1);
		}
	}
	
	return 0;
}

