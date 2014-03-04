#include "tbus.h"
#include "sip.h"
#include "bscp.h"

#include "tlog_log.h"
#include "tlog_print.h"



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
#define TBUS_MTU 65536



tbus_t *itb;
tbus_t *otb;

char *write_start = NULL;           //开始写入的指针
char *write_limit = NULL;           //结束写入的指针
char *write_cur = NULL;             //当前写入的指针

#define WRITE_LIMIT 1 * 1000000    //最大缓存长度

static void ts_flush()
{
    if(write_cur > write_start)
    {
//        ERROR_PRINT("ts_flush %u, %u, %d\n", otb->head_offset, otb->tail_offset, (write_cur - write_start));
        tbus_send_end(otb, (tbus_atomic_size_t)(write_cur - write_start));
        write_start = NULL;        
        write_cur = NULL;
        write_limit = NULL;
    }
}

static void ts_send(const sip_rsp_t *pkg, const char* data, size_t data_size)
{
    tbus_atomic_size_t total_size= 0;
    size_t head_size;

    head_size = SIP_RSP_T_CODE_SIZE(pkg);
    total_size = (tbus_atomic_size_t)(head_size + data_size);
      

    if(write_limit - write_cur < total_size)
    {
        tbus_atomic_size_t write_size;
        ts_flush();

        write_size = tbus_send_begin(otb, &write_start, total_size);
//        ERROR_PRINT("tbus_send_begin %u, %u, %u\n", otb->head_offset, otb->tail_offset, write_size);

        write_cur = write_start;        
        write_limit = write_cur + write_size;

        
        if(write_limit - write_cur < total_size)
        {
            ERROR_PRINT("tbus no space, drop pkg->cmd [%d] pkg->size [%u].", pkg->cmd, pkg->size);
            return;
        }
    }


    memcpy(write_cur, pkg, head_size);         
    sip_rsp_t_code((sip_rsp_t*)write_cur);
    write_cur += head_size;
    if(data)
    {
        memcpy(write_cur, data, data_size);
        write_cur += data_size;
    }

    
//    DEBUG_PRINT("ts_send pkg.cmd = %d, pkg.cid_list_num = %u, pkg.cid_list[0].id=%u, pkg.cid_list[0].sn=%"PRIu64" pkg.size = %u data_size = %zu."
//        , pkg->cmd, pkg->cid_list_num, pkg->cid_list[0].id, pkg->cid_list[0].sn, pkg->size, data_size);

    if(write_cur - write_start > WRITE_LIMIT)
    {
        ts_flush();
    }
}

#define BLOCK_SIZE 1024

static char buff[BLOCK_SIZE];

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
        ts_send(&rsp, NULL, 0);
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
                ts_send(&rsp, NULL, 0);
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
//                    int i;
                    
                    bscp_head_t pkg_size = *(const bscp_head_t*)iter;
//                    const char* pkg_content = iter + sizeof(bscp_head_t);

                    bscp_head_t_decode(pkg_size);
                    
                    next = iter + BSCP_HEAD_T_SIZE + pkg_size;                    
//                    DEBUG_PRINT("[%"PRIu64"] recv pkg_size: %u", req->cid.sn, pkg_size);
                    
//分10小块发也不会影响网络的收发效率
/*
                    for(i = 0; i < 10; ++i)
                    {
                        rsp.size = BLOCK_SIZE / 10;

                        ts_send(&rsp, buff, rsp.size);
                    }
*/
                    rsp.size = BLOCK_SIZE;
    
                    ts_send(&rsp, buff, rsp.size);

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
	tbus_atomic_size_t message_len = 0;
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
		message_len = tbus_read_begin(itb, &message);
        if(message_len == 0)
		{
			++idle_times;
			if(idle_times > 30)
			{
				idle_times = 0;
				usleep(100);
			}
			continue;
		}

        idle_times = 0;
        len = (size_t)message_len;
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
		tbus_read_end(itb, message_len);
        ts_flush();
		idle_times = 0;
	}
	
	return 0;
}

