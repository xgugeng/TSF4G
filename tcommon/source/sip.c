#include "sip.h"
#include <unistd.h>
#include <stdint.h>

ssize_t sip_rst_t_decode(char *buff_start, char *buff_limit)
{
    size_t i;
    sip_rsp_t *self = (sip_rsp_t*)buff_start;
    char *cur = (char*)buff_start;
    
    if(buff_limit - cur >= sizeof(self->cmd))
    {
        tlibc_little_to_host16(self->cmd);
        cur += sizeof(self->cmd);
    }
    else
    {
        return -1;
    }

    if(buff_limit - cur >= sizeof(self->size))
    {
        tlibc_little_to_host32(self->size);
        cur += sizeof(self->size);
    }
    else
    {
        return -1;
    }
    

    if(buff_limit - cur >= sizeof(self->cid_list_num))
    {
        tlibc_little_to_host32(self->cid_list_num);
        cur += sizeof(self->cid_list_num);
    }
    else
    {
        return -1;
    }

    if((buff_limit - cur) < (sizeof(sip_cid_t) * self->cid_list_num))
    {
        return -1;
    }

    for(i = 0; i < self->cid_list_num; ++i)
    {
        tlibc_little_to_host64(self->cid_list[i].sn);
        tlibc_little_to_host32(self->cid_list[i].id);        
    }
    cur += sizeof(sip_cid_t) * self->cid_list_num;

    return cur - buff_start;
}

