#include "tapp.h"
#include "tbus.h"
#include "sip.h"
#include "tconnd_proto.h"

#include "tlog_log.h"
#ifdef MAKE_RELEASE
#define DEBUG_PRINT_OFF
#define INFO_PRINT_OFF
#endif
#include "tlog_print.h"
#include "bscp_types.h"



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
#define WMEM_MAX 1 * 1000000    //最大缓存长度

typedef struct tconnapi_s
{
	int iid;
	tbus_t *itb;
	int oid;
	tbus_t *otb;

	char *write_start;           //开始写入的指针
	char *write_limit;           //结束写入的指针
	char *write_cur;            //当前写入的指针
	size_t wmem_max;
}tconnapi_t;

tconnapi_t g_tconn;

static void tconnapi_flush(tconnapi_t *self)
{
    if(self->write_cur > self->write_start)
    {
        tbus_send_end(self->otb, (tbus_atomic_size_t)(self->write_cur - self->write_start));
        self->write_start = NULL;        
        self->write_cur = NULL;
        self->write_limit = NULL;
    }
}

static void send_rsp(tconnapi_t *self, const sip_rsp_t *rsp, const char* data, size_t data_size)
{
    tbus_atomic_size_t total_size= 0;
    size_t head_size;

    head_size = SIZEOF_SIP_RSP_T(rsp);
    total_size = (tbus_atomic_size_t)(head_size + data_size);
      

	//todo total_size 靠靠靠
    if(self->write_limit - self->write_cur < total_size)
    {
        tbus_atomic_size_t write_size;
        tconnapi_flush(self);

        write_size = tbus_send_begin(self->otb, &self->write_start, total_size);

        self->write_cur = self->write_start;        
        self->write_limit = self->write_cur + write_size;

        
        if(self->write_limit - self->write_cur < total_size)
        {
            ERROR_PRINT("tbus no space, drop rsp->cmd [%d] rsp->size [%u].", rsp->cmd, rsp->size);
            return;
        }
    }


    memcpy(self->write_cur, rsp, head_size);         
    self->write_cur += head_size;
    if(data)
    {
        memcpy(self->write_cur, data, data_size);
        self->write_cur += data_size;
    }

    
    if(self->write_cur - self->write_start >= self->wmem_max)
    {
        tconnapi_flush(self);
    }
}

static void tconnd_send(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num, const char* buff, sip_size_t buff_size)
{
    sip_rsp_t rsp;

	rsp.cmd = e_sip_rsp_cmd_send;
    rsp.cid_list_num = cid_vec_num;
	memcpy(rsp.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
	rsp.size = buff_size; 
	send_rsp(self, &rsp, buff, rsp.size);
}

static void tconnd_close(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num)
{
    sip_rsp_t rsp;

    rsp.cmd = e_sip_rsp_cmd_close;
    rsp.cid_list_num = cid_vec_num;
	memcpy(rsp.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
    rsp.size = 0;
    send_rsp(self, &rsp, NULL, 0);
}

static void tconnd_accept(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num)
{
    sip_rsp_t rsp;

    rsp.cmd = e_sip_rsp_cmd_accept;
    rsp.cid_list_num = cid_vec_num;
	memcpy(rsp.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
    rsp.size = 0;
    send_rsp(self, &rsp, NULL, 0);
}

static void on_connect(tconnapi_t *self, const sip_cid_t *cid)
{
	tconnd_accept(self, cid, 1);
    INFO_PRINT("[%u, %"PRIu64"] accept.", cid->id, cid->sn);
}

static void on_close(tconnapi_t *self, const sip_cid_t *cid)
{
	INFO_PRINT("[%u, %"PRIu64"] client close.", cid->id, cid->sn);
}

static void on_recv(tconnapi_t *self, const sip_cid_t *cid, const char *packet, sip_size_t packet_size)
{
	const robot_proto_t *msg_req = (const robot_proto_t*)packet;
	robot_proto_t msg_rsp;

	if(packet_size != sizeof(robot_proto_t))
	{
		ERROR_PRINT("protocol decode failed.");
		tconnd_close(self, cid, 1);
		goto done;
	}

	msg_rsp.message_id = e_robot_login_rsp;
	memcpy(&msg_rsp.message_body.login_rsp.name, &msg_req->message_body.login_req.name, ROBOT_STR_LEN);
	msg_rsp.message_body.login_rsp.sid = (uint32_t)atoi(msg_req->message_body.login_req.pass);

	tconnd_send(self, cid, 1, (const char*)&msg_rsp, sizeof(robot_proto_t));
done:
	return;
}



static TERROR_CODE tconnapi_process(tconnapi_t *self)
{
    sip_req_t *req;
	tbus_atomic_size_t message_len = 0;
	char *message = NULL;

    message_len = tbus_read_begin(self->itb, &message);
    if(message_len == 0)
    {
		goto would_block;
    }

	if(message_len < sizeof(sip_req_t))
	{
        ERROR_PRINT("message_len < sizeof(sip_req_t)");
		goto bad_sip_req;
	}
	req = (sip_req_t*)message;
	if(sizeof(sip_req_t) + req->size != message_len)
	{
       	ERROR_PRINT("sizeof(sip_req_t) != message_len");
		goto bad_sip_req;
	}

    switch(req->cmd)
    {
    case e_sip_req_cmd_connect:
		on_connect(self, &req->cid);
		break;
    case e_sip_req_cmd_recv:
		if(req->size == 0)
		{
			on_close(self, &req->cid);
		}
		else
		{
			const char *iter, *limit, *next;
			iter = message + sizeof(sip_req_t);
			limit = iter + req->size;
			for(; iter < limit; iter = next)
			{
				const char *packet;
				bscp_head_t packet_size;
				packet = iter + sizeof(bscp_head_t);

			   	if(packet > limit)
				{
					ERROR_PRINT("bad packet head");
					goto bad_packet;
				}
				packet_size = *(const bscp_head_t*)iter;

				next = packet + packet_size;
				if(next > limit)
				{
					ERROR_PRINT("packet too long");
					goto bad_packet;
				}
				on_recv(self, &req->cid, packet, packet_size);
			}
		}
		break;
	default:
        ERROR_PRINT("unknow msg");
		goto bad_sip_req;
	}

bad_packet:	
    tbus_read_end(self->itb, message_len);
    
    return E_TS_NOERROR;
bad_sip_req:
    tbus_read_end(self->itb, message_len);
	return E_TS_NOERROR;
would_block:
	return E_TS_WOULD_BLOCK;
}

static TERROR_CODE tconnapi_init(tconnapi_t *self, key_t ikey, key_t okey, size_t wmem_max)
{
	TERROR_CODE ret = E_TS_NOERROR;

	self->iid = shmget(ikey, 0, 0666);
	if(self->iid == -1)
	{
		ret = E_TS_ERRNO;
		goto done;
	}
    self->itb = shmat(self->iid , NULL, 0);
	if((ssize_t)self->itb == -1)
	{
		ret = E_TS_ERRNO;
		goto done;
	}

	self->oid = shmget(okey, 0, 0666);
	if(self->oid == -1)
	{
		ret = E_TS_ERRNO;
		goto done;
	}
    self->otb = shmat(self->oid, NULL, 0);
	if((ssize_t)self->otb == -1)
	{
		ret = E_TS_ERRNO;
		goto done;
	}

	self->wmem_max = wmem_max;
	self->write_start = NULL;
	self->write_cur = NULL;
	self->write_limit = NULL;

done:
	return ret;
}

static TERROR_CODE process()
{
	TERROR_CODE ret = tconnapi_process(&g_tconn);
    tconnapi_flush(&g_tconn);
	return ret; 
}

int main()
{
	if(tconnapi_init(&g_tconn, iSHM_KEY, oSHM_KEY, WMEM_MAX) != E_TS_NOERROR)
	{
		ERROR_PRINT("tconnapi_init failed.");
		return 1;
	}

    return tapp_loop(process, TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL);
}

