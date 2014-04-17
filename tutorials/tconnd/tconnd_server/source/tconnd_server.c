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

typedef sip_size_t (*encode_t)(const void *self, char *start, char *limit);

static sip_size_t robot_proto_encode(const robot_proto_t *self, char *start, char *limit)
{
	if(limit - start < sizeof(robot_proto_t))
	{
		return 0;
	}

	memcpy(start, self, sizeof(robot_proto_t));

	return sizeof(robot_proto_t);
}

typedef struct tconnapi_s
{
	int iid;
	tbus_t *itb;
	int oid;
	tbus_t *otb;

	encode_t encode;
}tconnapi_t;

tconnapi_t g_tconn;

static void send_rsp(tconnapi_t *self, sip_rsp_t *rsp, const char* data)
{
    size_t total_size= 0;
    size_t head_size;
    tbus_atomic_size_t write_size;
    char *buf;

    head_size = SIZEOF_SIP_RSP_T(rsp);
    total_size = head_size + sizeof(robot_proto_t);
    
    write_size = tbus_send_begin(self->otb, &buf);
    if(write_size < total_size)
    {
        goto done;
    }
    
	if(data)
	{
		rsp->size = self->encode(data, buf + head_size, buf + write_size);
		//rsp->size == 0 means close the connection.
	}
	else
	{
		rsp->size = 0;
	}
	memcpy(buf, rsp, head_size); 
	tbus_send_end(self->otb, (tbus_atomic_size_t)(head_size + rsp->size));
	
done:
    return;
}

static void tconnd_send(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num, const void* data)
{
    sip_rsp_t rsp;

	rsp.cmd = e_sip_rsp_cmd_send;
    rsp.cid_list_num = cid_vec_num;
	memcpy(rsp.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
	send_rsp(self, &rsp, data);
}

static void tconnd_close(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num)
{
    sip_rsp_t rsp;

    rsp.cmd = e_sip_rsp_cmd_close;
    rsp.cid_list_num = cid_vec_num;
	memcpy(rsp.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
    send_rsp(self, &rsp, NULL);
}

static void tconnd_accept(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num)
{
    sip_rsp_t rsp;

    rsp.cmd = e_sip_rsp_cmd_accept;
    rsp.cid_list_num = cid_vec_num;
	memcpy(rsp.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
    send_rsp(self, &rsp, NULL);
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

	tconnd_send(self, cid, 1, &msg_rsp);
done:
	return;
}

static TERROR_CODE tconnapi_process(tconnapi_t *self)
{
    TERROR_CODE ret = E_TS_NOERROR;
    sip_req_t *req;
	tbus_atomic_size_t message_len = 0;
    struct iovec iov[1];
    size_t iov_num;
	char *message = NULL;
	tbus_atomic_size_t tbus_head;

    iov_num = 1;

    tbus_head = tbus_read_begin(self->itb, iov, &iov_num);
    if(iov_num == 0)
    {
        if(tbus_head != self->itb->head_offset)
        {        
            goto read_end;
        }
        else
        {
            ret = E_TS_WOULD_BLOCK;
            goto done;
        }
    }

    message = iov[0].iov_base;
    message_len = (tbus_atomic_size_t)iov[0].iov_len;

	if(message_len < sizeof(sip_req_t))
	{
        ERROR_PRINT("message_len < sizeof(sip_req_t)");
		goto read_end;
	}
	req = (sip_req_t*)message;
	if(sizeof(sip_req_t) + req->size != message_len)
	{
       	ERROR_PRINT("sizeof(sip_req_t) != message_len");
		goto read_end;
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
					goto read_end;
				}
				packet_size = *(const bscp_head_t*)iter;

				next = packet + packet_size;
				if(next > limit)
				{
					ERROR_PRINT("packet too long");
					goto read_end;
				}
				on_recv(self, &req->cid, packet, packet_size);
			}
		}
		break;
	default:
        ERROR_PRINT("unknow msg");
		goto read_end;
	}

read_end:
    tbus_read_end(self->itb, tbus_head);
done:
	return ret;
}

static TERROR_CODE tconnapi_init(tconnapi_t *self, key_t ikey, key_t okey, encode_t encode)
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

	self->encode = encode;

done:
	return ret;
}

int main()
{
	if(tconnapi_init(&g_tconn, iSHM_KEY, oSHM_KEY, (encode_t)robot_proto_encode) != E_TS_NOERROR)
	{
		ERROR_PRINT("tconnapi_init failed.");
		return 1;
	}

    return tapp_loop(TAPP_IDLE_USEC, TAPP_IDLE_LIMIT, NULL, NULL, NULL, NULL
                     , tconnapi_process, &g_tconn
                     , NULL, NULL);
}

