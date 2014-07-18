#include "tconnapi.h"
#include "tbus.h"
#include "bscp_types.h"
#include "tlibcdef.h"

#include <sys/shm.h>
#include <string.h>

typedef struct tconnapi_rsp_s
{
    sip_rsp_t head;
    const char* body;
   	encode_t encode;
}tconnapi_rsp_t;

static tbus_atomic_size_t tconnapi_encode(char *dst, size_t dst_len, const char *src, size_t src_len)
{
    const tconnapi_rsp_t *rsp = (const tconnapi_rsp_t *)src;
    tbus_atomic_size_t head_size = (tbus_atomic_size_t)SIZEOF_SIP_RSP_T(&rsp->head);
    sip_rsp_t *head = NULL;
   
    if(dst_len < head_size)
    {
        goto error;
    }

    memcpy(dst, &rsp->head, head_size);
    head = (sip_rsp_t*)dst;
    
    if(rsp->body)
    {
        head->size = rsp->encode(rsp->body, dst + head_size, dst + dst_len);
        if(head->size == 0)
        {
            goto error;
        }
    }
    else
    {
        head->size = 0;
    }

    return head_size + head->size;
error:
    //close connection
    return 0;
}

void tconnapi_send(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num, const void* data)
{
    tconnapi_rsp_t rsp;
    rsp.body = data;
    rsp.encode = self->encode;

    
	rsp.head.cmd = e_sip_rsp_cmd_send;
    rsp.head.cid_list_num = cid_vec_num;
	memcpy(rsp.head.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
	tbusapi_send(&self->tbusapi, (const char*)&rsp, sizeof(tconnapi_rsp_t));
}

void tconnapi_close(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num)
{
    tconnapi_rsp_t rsp;
    rsp.body = NULL;
    rsp.encode = self->encode;

    rsp.head.cmd = e_sip_rsp_cmd_close;
    rsp.head.cid_list_num = cid_vec_num;
	memcpy(rsp.head.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
	tbusapi_send(&self->tbusapi, (const char*)&rsp, sizeof(tconnapi_rsp_t));
}

void tconnapi_accept(tconnapi_t *self, const sip_cid_t *cid_vec, uint16_t cid_vec_num)
{
    tconnapi_rsp_t rsp;

    rsp.body = NULL;
    rsp.encode = self->encode;

    rsp.head.cmd = e_sip_rsp_cmd_accept;
    rsp.head.cid_list_num = cid_vec_num;
	memcpy(rsp.head.cid_list, cid_vec, sizeof(sip_cid_t) * cid_vec_num);
	tbusapi_send(&self->tbusapi, (const char*)&rsp, sizeof(tconnapi_rsp_t));
}

static void tconnapi_on_recv(tbusapi_t *super, const char *buf, size_t buf_len)
{
    tconnapi_t *self = TLIBC_CONTAINER_OF(super, tconnapi_t, tbusapi);
    const sip_req_t *req = (const sip_req_t *)buf;
    
	if(buf_len < sizeof(sip_req_t))
	{
        goto done;
	}

	if(sizeof(sip_req_t) + req->size > buf_len)
	{
	    goto done;
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
			iter = buf + sizeof(sip_req_t);
			limit = iter + req->size;
			for(; iter < limit; iter = next)
			{
				const char *packet;
				bscp_head_t packet_size;
				packet = iter + sizeof(bscp_head_t);

			   	if(packet > limit)
				{
					goto done;
				}
				packet_size = *(const bscp_head_t*)iter;

				next = packet + packet_size;
				if(next > limit)
				{
					goto done;
				}
				if(self->on_recv)
				{
					self->on_recv(self, &req->cid, packet, packet_size);
				}
			}
		}
		break;
	default:
        goto done;
	}
	
done:
    return;
}

tlibc_error_code_t tconnapi_process(tconnapi_t *self)
{
    return tbusapi_process(&self->tbusapi);
}

tlibc_error_code_t tconnapi_init(tconnapi_t *self, key_t ikey, key_t okey, encode_t encode)
{
	tlibc_error_code_t ret = E_TLIBC_NOERROR;

	ret = tbusapi_init(&self->tbusapi, ikey, okey);
	if(ret != E_TLIBC_NOERROR)
	{
	    goto done;
	}
	
	self->tbusapi.on_recv = tconnapi_on_recv;
	self->tbusapi.encode = tconnapi_encode;

	self->encode = encode;
	self->on_connect = NULL;
	self->on_close = NULL;
	self->on_recv = NULL;

done:
	return ret;
}

