#include "tconnapi.h"
#include "tlog_print.h"
#include "tbus.h"
#include "bscp_types.h"

#include <sys/shm.h>
#include <string.h>

static void send_rsp(tconnapi_t *self, sip_rsp_t *rsp, const char* data)
{
    size_t head_size;
    tbus_atomic_size_t write_size;
    char *buf;

    head_size = SIZEOF_SIP_RSP_T(rsp);
    write_size = tbus_send_begin(self->otb, &buf);
	if(write_size < head_size)
	{
		goto done;
	}

	if(data)
	{
		rsp->size = self->encode(data, buf + head_size, buf + write_size);
		if(rsp->size == 0)
		{
			goto done;
		}
	}
	else
	{
		rsp->size = 0;
	}
	if(write_size < head_size + rsp->size)
	{
		goto done;
	}

	memcpy(buf, rsp, head_size); 
	tbus_send_end(self->otb, (tbus_atomic_size_t)(head_size + rsp->size));
	
done:
    return;
}

void tconnapi_send(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num, const void* data)
{
    sip_rsp_t rsp;

	rsp.cmd = e_sip_rsp_cmd_send;
    rsp.cid_list_num = cid_vec_num;
	memcpy(rsp.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
	send_rsp(self, &rsp, data);
}

void tconnapi_close(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num)
{
    sip_rsp_t rsp;

    rsp.cmd = e_sip_rsp_cmd_close;
    rsp.cid_list_num = cid_vec_num;
	memcpy(rsp.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
    send_rsp(self, &rsp, NULL);
}

void tconnapi_accept(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num)
{
    sip_rsp_t rsp;

    rsp.cmd = e_sip_rsp_cmd_accept;
    rsp.cid_list_num = cid_vec_num;
	memcpy(rsp.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
    send_rsp(self, &rsp, NULL);
}

TERROR_CODE tconnapi_process(tconnapi_t *self)
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
		if(self->on_connect)
		{
			self->on_connect(self, &req->cid);
		}
		break;
    case e_sip_req_cmd_recv:
		if(req->size == 0)
		{
			if(self->on_close)
			{
				self->on_close(self, &req->cid);
			}
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
				if(self->on_recv)
				{
					self->on_recv(self, &req->cid, packet, packet_size);
				}
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

TERROR_CODE tconnapi_init(tconnapi_t *self, key_t ikey, key_t okey, encode_t encode)
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
	self->on_connect = NULL;
	self->on_close = NULL;
	self->on_recv = NULL;

done:
	return ret;
}

